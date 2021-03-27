#include "gdsqlite.h"

using namespace godot;

void SQLite::_register_methods()
{

    register_method("open_db", &SQLite::open_db);
    register_method("close_db", &SQLite::close_db);
    register_method("query", &SQLite::query);
    register_method("query_with_bindings", &SQLite::query_with_bindings);

    register_method("create_table", &SQLite::create_table);
    register_method("drop_table", &SQLite::drop_table);

    register_method("insert_row", &SQLite::insert_row);
    register_method("insert_rows", &SQLite::insert_rows);

    register_method("select_rows", &SQLite::select_rows);
    register_method("update_rows", &SQLite::update_rows);
    register_method("delete_rows", &SQLite::delete_rows);

    register_method("create_function", &SQLite::create_function);

    register_method("import_from_json", &SQLite::import_from_json);
    register_method("export_to_json", &SQLite::export_to_json);

    register_property<SQLite, int>("last_insert_rowid", &SQLite::set_last_insert_rowid, &SQLite::get_last_insert_rowid, 0);

    register_property<SQLite, bool>("verbose_mode", &SQLite::verbose_mode, false);
    register_property<SQLite, bool>("foreign_keys", &SQLite::foreign_keys, false);

    register_property<SQLite, String>("path", &SQLite::path, "default");
    register_property<SQLite, String>("error_message", &SQLite::error_message, "");

    register_property<SQLite, Array>("query_result", &SQLite::query_result, Array());
}

SQLite::SQLite()
{
    db = nullptr;
    query_result = Array();
}

SQLite::~SQLite()
{
    /* Clean up the function_registry */
    function_registry.clear();
    function_registry.shrink_to_fit();
    close_db();
}

void SQLite::_init()
{
    path = String("default");
    verbose_mode = false;
    foreign_keys = false;
}

bool SQLite::open_db()
{
    char *zErrMsg = 0;
    int rc;
    if (path != ":memory:")
    {
        /* Add the ".db"-extension to the database path if not already present */
        String ending = String(".db");
        if (!path.ends_with(ending))
        {
            path += ending;
        }

        /* Find the real path */
        path = ProjectSettings::get_singleton()->globalize_path(path.strip_edges());
        /* This part does weird things on Android & on export! Leave it out for now! */
        ///* Make the necessary empty directories if they do not exist yet */
        //Ref<Directory> dir = Directory::_new();
        //PoolStringArray split_array = path.split("/", false);
        ///* Remove the database filename */
        //split_array.remove(split_array.size()-1);
        //String path_without_file = "";
        //for (int i = 0; i < split_array.size(); i++) {
        //    path_without_file += split_array[i] + "/";
        //}
        //Error error = dir->make_dir_recursive(path_without_file);
        //if (error != Error::OK){
        //    GODOT_LOG(2, "GDSQLite Error: Can't make necessary folders for path (ERROR = "
        //    + String(std::to_string(static_cast<int>(error)).c_str())
        //    + ")")
        //    return false;
        //}
    }

    const char *char_path = path.alloc_c_string();
    /* Try to open the database */
    rc = sqlite3_open(char_path, &db);
    if (rc != SQLITE_OK)
    {
        GODOT_LOG(2, "GDSQLite Error: Can't open database: " + String(sqlite3_errmsg(db)))
        return false;
    }
    else
    {
        GODOT_LOG(0, "Opened database successfully (" + path + ")")
    }

    /* Try to enable foreign keys. */
    if (foreign_keys)
    {
        rc = sqlite3_exec(db, "PRAGMA foreign_keys=on;", NULL, NULL, &zErrMsg);
        if (rc != SQLITE_OK)
        {
            GODOT_LOG(2, "GDSQLite Error: Can't enable foreign keys: " + String(zErrMsg))
            sqlite3_free(zErrMsg);
            return false;
        }
    }

    return true;
}

void SQLite::close_db()
{
    if (db)
    {
        // Cannot close database!
        if (sqlite3_close_v2(db) != SQLITE_OK)
        {
            GODOT_LOG(2, "GDSQLite Error: Can't close database!")
        }
        else
        {
            db = nullptr;
            GODOT_LOG(0, "Closed database (" + path + ")")
        }
    }
}

bool SQLite::query(String p_query)
{
    return query_with_bindings(p_query, Array());
}

bool SQLite::query_with_bindings(String p_query, Array param_bindings)
{
    const char *zErrMsg, *sql;
    int rc;

    if (verbose_mode)
    {
        GODOT_LOG(0, p_query)
    }
    sql = p_query.alloc_c_string();

    /* Clear the previous query results */
    query_result.clear();

    sqlite3_stmt *stmt;
    /* Prepare an SQL statement */
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    zErrMsg = sqlite3_errmsg(db);
    error_message = String(zErrMsg);
    if (rc != SQLITE_OK)
    {
        GODOT_LOG(2, " --> SQL error: " + error_message)
        return false;
    }

    /* Bind any given parameters to the prepared statement */
    for (int i = 0; i < param_bindings.size(); i++)
    {
        switch (param_bindings[i].get_type())
        {
        case Variant::NIL:
            sqlite3_bind_null(stmt, i + 1);
            break;

        case Variant::BOOL:
        case Variant::INT:
            sqlite3_bind_int64(stmt, i + 1, int64_t(param_bindings[i]));
            break;

        case Variant::REAL:
            sqlite3_bind_double(stmt, i + 1, param_bindings[i]);
            break;

        case Variant::STRING:
            sqlite3_bind_text(stmt, i + 1, (param_bindings[i].operator String()).alloc_c_string(), -1, SQLITE_TRANSIENT);
            break;

        case Variant::POOL_BYTE_ARRAY:
        {
            PoolByteArray binding = ((const PoolByteArray &)param_bindings[i]);
            PoolByteArray::Read r = binding.read();
            sqlite3_bind_blob64(stmt, i + 1, r.ptr(), binding.size(), SQLITE_TRANSIENT);
            break;
        }

        default:
            GODOT_LOG(2, "GDSQLite Error: Binding a parameter of type " + String(std::to_string(param_bindings[i].get_type()).c_str()) + " (TYPE_*) is not supported!")
            sqlite3_finalize(stmt);
            return false;
        }
    }

    if (verbose_mode)
    {
        char *expanded_sql = sqlite3_expanded_sql(stmt);
        GODOT_LOG(0, expanded_sql)
        sqlite3_free(expanded_sql);
    }

    // Execute the statement and iterate over all the resulting rows.
    while (sqlite3_step(stmt) == SQLITE_ROW)
    {
        Dictionary column_dict;
        int argc = sqlite3_column_count(stmt);

        /* Loop over all columns and add them to the Dictionary */
        for (int i = 0; i < argc; i++)
        {
            Variant column_value;
            /* Check the column type and do correct casting */
            switch (sqlite3_column_type(stmt, i))
            {
            case SQLITE_INTEGER:
                column_value = Variant((int64_t)sqlite3_column_int64(stmt, i));
                break;

            case SQLITE_FLOAT:
                column_value = Variant(sqlite3_column_double(stmt, i));
                break;

            case SQLITE_TEXT:
                column_value = Variant((char *)sqlite3_column_text(stmt, i));
                break;

            case SQLITE_BLOB:
            {
                int bytes = sqlite3_column_bytes(stmt, i);
                PoolByteArray arr = PoolByteArray();
                arr.resize(bytes);
                PoolByteArray::Write write = arr.write();
                memcpy(write.ptr(), (char *)sqlite3_column_blob(stmt, i), bytes);
                column_value = arr;
                break;
            }

            case SQLITE_NULL:
                break;

            default:
                break;
            }

            const char *azColName = sqlite3_column_name(stmt, i);
            column_dict[String(azColName)] = column_value;
        }
        /* Add result to query_result Array */
        query_result.append(column_dict);
    }

    // Clean up and delete the resources used by the prepared statement.
    sqlite3_finalize(stmt);

    rc = sqlite3_errcode(db);
    zErrMsg = sqlite3_errmsg(db);
    error_message = String(zErrMsg);
    if (rc != SQLITE_OK)
    {
        GODOT_LOG(2, " --> SQL error: " + error_message)
        return false;
    }
    else if (verbose_mode)
    {
        GODOT_LOG(0, " --> Query succeeded")
    }

    return true;
}

bool SQLite::create_table(String p_name, Dictionary p_table_dict)
{
    String query_string, type_string, key_string;
    String integer_datatype = "int";
    /* Create SQL statement */
    query_string = "CREATE TABLE IF NOT EXISTS " + p_name + " (";
    key_string = "";

    Dictionary column_dict;
    Array columns = p_table_dict.keys();
    int number_of_columns = columns.size();
    for (int i = 0; i <= number_of_columns - 1; i++)
    {
        column_dict = p_table_dict[columns[i]];
        if (!column_dict.has("data_type"))
        {
            GODOT_LOG(2, "GDSQLite Error: The field \"data_type\" is a required part of the table dictionary")
            return false;
        }
        query_string += (const String &)columns[i] + " ";
        type_string = (const String &)column_dict["data_type"];
        if (type_string.to_lower().begins_with(integer_datatype))
        {
            query_string += String("INTEGER");
        }
        else
        {
            query_string += type_string;
        }
        /* To be cleaned up whenever godot-cpp receives decent Dictionary get() with default... */
        /* Primary key check */
        if (get_with_default(column_dict, "primary_key", false))
        {
            query_string += String(" PRIMARY KEY");
        }
        /* Default check */
        if (column_dict.has("default"))
        {
            query_string += String(" DEFAULT ") + (const String &)column_dict["default"];
        }
        /* Autoincrement check */
        if (get_with_default(column_dict, "auto_increment", false))
        {
            query_string += String(" AUTOINCREMENT");
        }
        /* Not null check */
        if (get_with_default(column_dict, "not_null", false))
        {
            query_string += String(" NOT NULL");
        }
        /* Apply foreign key constraint. */
        if (foreign_keys)
        {
            if (get_with_default(column_dict, "foreign_key", false))
            {
                const String foreign_key_definition = (const String &)(column_dict["foreign_key"]);
                const Array foreign_key_elements = foreign_key_definition.split(".");
                if (foreign_key_elements.size() == 2)
                {
                    const String column_name = (const String &)(columns[i]);
                    const String foreign_key_table_name = (const String &)(foreign_key_elements[0]);
                    const String foreign_key_column_name = (const String &)(foreign_key_elements[1]);
                    key_string += String(", FOREIGN KEY (" + column_name + ") REFERENCES " + foreign_key_table_name + "(" + foreign_key_column_name + ")");
                }
            }
        }

        if (i != number_of_columns - 1)
        {
            query_string += ",";
        }
    }

    query_string += key_string + ");";

    return query(query_string);
}

bool SQLite::drop_table(String p_name)
{
    String query_string;
    /* Create SQL statement */
    query_string = "DROP TABLE " + p_name + ";";

    return query(query_string);
}

bool SQLite::insert_row(String p_name, Dictionary p_row_dict)
{
    String query_string, key_string, value_string = "";
    Array keys = p_row_dict.keys();
    Array param_bindings = p_row_dict.values();

    /* Create SQL statement */
    query_string = "INSERT INTO " + p_name;

    int number_of_keys = p_row_dict.size();
    for (int i = 0; i <= number_of_keys - 1; i++)
    {
        key_string += (const String &)keys[i];
        value_string += "?";
        if (i != number_of_keys - 1)
        {
            key_string += ",";
            value_string += ",";
        }
    }
    query_string += " (" + key_string + ") VALUES (" + value_string + ");";

    return query_with_bindings(query_string, param_bindings);
}

bool SQLite::insert_rows(String p_name, Array p_row_array)
{
    query("BEGIN TRANSACTION;");
    int number_of_rows = p_row_array.size();
    for (int i = 0; i <= number_of_rows - 1; i++)
    {
        if (p_row_array[i].get_type() != Variant::DICTIONARY)
        {
            GODOT_LOG(2, "GDSQLite Error: All elements of the Array should be of type Dictionary")
            return false;
        }
        if (!insert_row(p_name, p_row_array[i]))
        {
            /* Don't forget to close the transaction! */
            /* Stop the error_message from being overwritten! */
            String previous_error_message = error_message;
            query("END TRANSACTION;");
            error_message = previous_error_message;
            return false;
        }
    }
    query("END TRANSACTION;");
    return true;
}

Array SQLite::select_rows(String p_name, String p_conditions, Array p_columns_array)
{
    String query_string;
    /* Create SQL statement */
    query_string = "SELECT ";

    int number_of_columns = p_columns_array.size();
    for (int i = 0; i <= number_of_columns - 1; i++)
    {
        if (p_columns_array[i].get_type() != Variant::STRING)
        {
            GODOT_LOG(2, "GDSQLite Error: All elements of the Array should be of type String")
            return query_result;
        }
        query_string += (const String &)p_columns_array[i];

        if (i != number_of_columns - 1)
        {
            query_string += ", ";
        }
    }
    query_string += " FROM " + p_name;
    if (!p_conditions.empty())
    {
        query_string += " WHERE " + p_conditions;
    }
    query_string += ";";

    query(query_string);
    return query_result;
}

bool SQLite::update_rows(String p_name, String p_conditions, Dictionary p_updated_row_dict)
{
    String query_string;
    Array param_bindings;
    bool success;

    int number_of_keys = p_updated_row_dict.size();
    Array keys = p_updated_row_dict.keys();
    Array values = p_updated_row_dict.values();

    query("BEGIN TRANSACTION;");
    /* Create SQL statement */
    query_string += "UPDATE " + p_name + " SET ";

    for (int i = 0; i <= number_of_keys - 1; i++)
    {
        query_string += (const String &)keys[i] + "=?";
        param_bindings.append(values[i]);
        if (i != number_of_keys - 1)
        {
            query_string += ", ";
        }
    }
    query_string += " WHERE " + p_conditions + ";";

    success = query_with_bindings(query_string, param_bindings);
    /* Stop the error_message from being overwritten! */
    String previous_error_message = error_message;
    query("END TRANSACTION;");
    error_message = previous_error_message;
    return success;
}

bool SQLite::delete_rows(String p_name, String p_conditions)
{
    String query_string;
    bool success;

    query("BEGIN TRANSACTION;");
    /* Create SQL statement */
    query_string = "DELETE FROM " + p_name;
    /* If it's empty or * everything is to be deleted */
    if (!p_conditions.empty() && (p_conditions != (const String &)"*"))
    {
        query_string += " WHERE " + p_conditions;
    }
    query_string += ";";

    success = query(query_string);
    /* Stop the error_message from being overwritten! */
    String previous_error_message = error_message;
    query("END TRANSACTION;");
    error_message = previous_error_message;
    return success;
}

static void function_callback(sqlite3_context *context, int argc, sqlite3_value **argv)
{
    void *temp = sqlite3_user_data(context);
    Ref<FuncRef> func_ref = *(Ref<FuncRef> *)&temp;
    /* Can also be done with following single-line statement, but I prefer the above */
    /* Ref<FuncRef> func_ref = reinterpret_cast<Ref<FuncRef> >(sqlite3_user_data(context)); */

    /* Check validity of the function reference */
    if (!func_ref->is_valid())
    {
        GODOT_LOG(2, "GDSQLite Error: Supplied function reference is invalid! Aborting callback...")
        return;
    }

    Array argument_array = Array();
    Variant argument_value;
    for (int i = 0; i <= argc - 1; i++)
    {
        sqlite3_value *value = *argv;
        /* Check the value type and do correct casting */
        switch (sqlite3_value_type(value))
        {
        case SQLITE_INTEGER:
            argument_value = Variant((int64_t)sqlite3_value_int64(value));
            break;

        case SQLITE_FLOAT:
            argument_value = Variant(sqlite3_value_double(value));
            break;

        case SQLITE_TEXT:
            argument_value = Variant((char *)sqlite3_value_text(value));
            break;

        case SQLITE_BLOB:
        {
            int bytes = sqlite3_value_bytes(value);
            PoolByteArray arr = PoolByteArray();
            arr.resize(bytes);
            PoolByteArray::Write write = arr.write();
            memcpy(write.ptr(), (char *)sqlite3_value_blob(value), bytes);
            argument_value = arr;
            break;
        }

        case SQLITE_NULL:
            break;

        default:
            break;
        }

        argument_array.append(argument_value);
        argv += 1;
    }

    Variant output = func_ref->call_func(argument_array);

    switch (output.get_type())
    {
    case Variant::NIL:
        sqlite3_result_null(context);
        break;

    case Variant::BOOL:
    case Variant::INT:
        sqlite3_result_int64(context, int64_t(output));
        break;

    case Variant::REAL:
        sqlite3_result_double(context, output);
        break;

    case Variant::STRING:
        sqlite3_result_text(context, (output.operator String()).alloc_c_string(), -1, SQLITE_STATIC);
        break;

    case Variant::POOL_BYTE_ARRAY:
    {
        PoolByteArray arr = ((const PoolByteArray &)output);
        PoolByteArray::Read r = arr.read();
        sqlite3_result_blob(context, r.ptr(), arr.size(), SQLITE_TRANSIENT);
        break;
    }

    default:
        break;
    }
}

bool SQLite::create_function(String p_name, Ref<FuncRef> p_func_ref, int p_argc)
{
    /* Add the func_ref to a std::vector to increase the ref_count */
    function_registry.push_back(p_func_ref);

    int rc;
    const char *zFunctionName = p_name.alloc_c_string();
    int nArg = p_argc;
    int eTextRep = SQLITE_UTF8;

    /* Get a void pointer to the current value stored at the back of the vector */
    void *pApp = *(void **)&function_registry.back();
    void (*xFunc)(sqlite3_context *, int, sqlite3_value **) = function_callback;
    void (*xStep)(sqlite3_context *, int, sqlite3_value **) = NULL;
    void (*xFinal)(sqlite3_context *) = NULL;

    /* Create the actual function */
    rc = sqlite3_create_function(db, zFunctionName, nArg, eTextRep, pApp, xFunc, xStep, xFinal);
    if (rc)
    {
        GODOT_LOG(2, "GDSQLite Error: " + String(sqlite3_errmsg(db)))
        return false;
    }
    else if (verbose_mode)
    {
        GODOT_LOG(0, "Succesfully added function \"" + p_name + "\" to function registry")
    }
    return true;
}

bool SQLite::import_from_json(String import_path)
{
    /* Add .json to the import_path String if not present */
    String ending = String(".json");
    if (!import_path.ends_with(ending))
    {
        import_path += ending;
    }
    /* Find the real path */
    import_path = ProjectSettings::get_singleton()->globalize_path(import_path.strip_edges());
    const char *char_path = import_path.alloc_c_string();

    /* Open the json-file and stream its content into a stringstream */
    std::ifstream ifs(char_path);
    if (ifs.fail())
    {
        GODOT_LOG(2, "GDSQLite Error: Failed to open specified json-file (" + import_path + ")")
        return false;
    }
    std::stringstream buffer;
    buffer << ifs.rdbuf();
    std::string str = buffer.str();
    String json_string = String(str.c_str());
    ifs.close();

    /* Attempt to parse the result and, if unsuccessful, throw a parse error specifying the erroneous line */
    Ref<JSONParseResult> result = JSON::get_singleton()->parse(json_string);
    if (result->get_error() != Error::OK)
    {
        /* Throw a parsing error */
        GODOT_LOG(2, "GDSQLite Error: parsing failed! reason: " + result->get_error_string() + ", at line: " + String::num_int64(result->get_error_line()))
        return false;
    }
    Array database_array = result->get_result();
    std::vector<object_struct> objects_to_import;
    /* Validate the json structure and populate the tables_to_import vector */
    if (!validate_json(database_array, objects_to_import))
    {
        return false;
    }

    /* Check if the database is open and, if not, attempt to open it */
    if (db == nullptr)
    {
        /* Open the database using the open_db method */
        if (!open_db()){
            return false;
        }
    }

    /* Find all tables that are present in this database */
    /* We don't care about triggers here since they get dropped automatically when their table is dropped */
    query(String("SELECT name FROM sqlite_master WHERE type = 'table';"));
    Array old_database_array = query_result.duplicate(true);
    int old_number_of_tables = query_result.size();
    /* Drop all old tables present in the database */
    for (int i = 0; i <= old_number_of_tables - 1; i++)
    {
        Dictionary table_dict = old_database_array[i];
        String table_name = table_dict["name"];

        drop_table(table_name);
    }

    query("BEGIN TRANSACTION;");
    /* foreign_keys cannot be enforced until after all rows have been added! */
    query("PRAGMA defer_foreign_keys=on;");
    /* Add all new tables and fill them up with all the rows */
    for (auto table : objects_to_import)
    {
        if (!query(table.sql))
        {
            /* Don't forget to close the transaction! */
            /* Stop the error_message from being overwritten! */
            String previous_error_message = error_message;
            query("END TRANSACTION;");
            error_message = previous_error_message;
        }
    }

    for (auto object : objects_to_import)
    {
        if (object.type != TABLE)
        {
            /* The object is a trigger and doesn't have any rows! */
            continue;
        }

        /* Convert the base64-encoded columns back to raw data */
        for (int i = 0; i <= object.base64_columns.size() - 1; i++)
        {
            String key = object.base64_columns[i];
            for (int j = 0; j <= object.row_array.size() - 1; j++)
            {
                Dictionary row = object.row_array[j];

                if (row.has(key))
                {
                    std::string input = ((const String &)row[key]).alloc_c_string();
                    std::string output;
                    macaron::Base64::Decode(input, output);

                    PoolByteArray arr = PoolByteArray();
                    arr.resize(output.length());
                    PoolByteArray::Write write = arr.write();
                    memcpy(write.ptr(), output.data(), output.length());

                    row[key] = arr;
                }
            }
        }

        int number_of_rows = object.row_array.size();
        for (int i = 0; i <= number_of_rows - 1; i++)
        {
            if (object.row_array[i].get_type() != Variant::DICTIONARY)
            {
                GODOT_LOG(2, "GDSQLite Error: All elements of the Array should be of type Dictionary")
                return false;
            }
            if (!insert_row(object.name, object.row_array[i]))
            {
                /* Don't forget to close the transaction! */
                /* Stop the error_message from being overwritten! */
                String previous_error_message = error_message;
                query("END TRANSACTION;");
                error_message = previous_error_message;
                return false;
            }
        }
    }
    query("END TRANSACTION;");
    return true;
}

bool SQLite::export_to_json(String export_path)
{
    /* Get all names and sql templates for all tables present in the database */
    query(String("SELECT name,sql,type FROM sqlite_master WHERE type = 'table' OR type = 'trigger';"));
    int number_of_objects = query_result.size();
    Array database_array = query_result.duplicate(true);
    /* Construct a Dictionary for each table, convert it to JSON and write it to file */
    for (int i = 0; i <= number_of_objects - 1; i++)
    {
        Dictionary object_dict = database_array[i];

        if (object_dict["type"] == String("table")){
            String object_name = object_dict["name"];
            String query_string;

            query_string = "SELECT * FROM " + (const String &)object_name + ";";
            query(query_string);

            /* Encode all columns of type PoolByteArray to base64 */
            if (!query_result.empty())
            {
                /* First identify the columns that are of this type! */
                Array base64_columns = Array();
                Dictionary initial_row = query_result[0];
                Array keys = initial_row.keys();
                for (int k = 0; k <= keys.size() - 1; k++)
                {
                    String key = keys[k];
                    Variant value = initial_row[key];
                    if (value.get_type() == Variant::POOL_BYTE_ARRAY)
                    {
                        base64_columns.append(key);
                    }
                }

                /* Now go through all the rows and encode the relevant columns */
                for (int k = 0; k <= base64_columns.size() - 1; k++)
                {
                    String key = base64_columns[k];
                    for (int j = 0; j <= query_result.size() - 1; j++)
                    {
                        Dictionary row = query_result[j];
                        PoolByteArray arr = ((const PoolByteArray &)row[key]);
                        PoolByteArray::Read r = arr.read();

                        std::string s(r.ptr(), r.ptr() + arr.size());
                        std::string encoded_string = macaron::Base64::Encode(s);

                        /* Has to be erased to make sure PoolByteArray is cleaned up properly!!!*/
                        row.erase(key);
                        row[key] = godot::String(encoded_string.c_str());
                    }
                }

                if (!base64_columns.empty()){
                    object_dict["base64_columns"] = base64_columns;
                }
            }
            object_dict["row_array"] = query_result.duplicate(true);

        }
    }

    /* Add .json to the import_path String if not present */
    String ending = String(".json");
    if (!export_path.ends_with(ending))
    {
        export_path += ending;
    }
    /* Find the real path */
    export_path = ProjectSettings::get_singleton()->globalize_path(export_path.strip_edges());
    const char *char_path = export_path.alloc_c_string();

    std::ofstream ofs(char_path, std::ios::trunc);
    if (ofs.fail())
    {
        GODOT_LOG(2, "GDSQLite Error: Can't open specified json-file, file does not exist or is locked")
        return false;
    }
    String json_string = JSON::get_singleton()->print(database_array, "\t");
    ofs << json_string.alloc_c_string();
    ofs.close();

    return true;
}

void SQLite::set_last_insert_rowid(int p_last_row_id)
{
    GODOT_LOG(2, "GDSQlite Error: Variable `last_insert_rowid` cannot be set externally.")
}

int SQLite::get_last_insert_rowid()
{
    return sqlite3_last_insert_rowid(db);
}

bool SQLite::validate_json(Array database_array, std::vector<object_struct> &objects_to_import)
{
    /* Start going through all the tables and checking their validity */
    int number_of_objects = database_array.size();
    for (int i = 0; i <= number_of_objects - 1; i++)
    {
        /* Create a new object_struct */
        object_struct new_object;

        Dictionary temp_dict = database_array[i];
        /* Get the name of the object */
        if (!temp_dict.has("name"))
        {
            /* Did not find the necessary key! */
            GODOT_LOG(2, "GDSQlite Error: Did not find required key \"name\" in the supplied json-file")
            return false;
        }
        new_object.name = temp_dict["name"];

        /* Extract the sql template for generating the object */
        if (!temp_dict.has("sql"))
        {
            /* Did not find the necessary key! */
            GODOT_LOG(2, "GDSQlite Error: Did not find required key \"sql\" in the supplied json-file")
            return false;
        }
        new_object.sql = temp_dict["sql"];

        if (!temp_dict.has("type"))
        {
            /* Did not find the necessary key! */
            GODOT_LOG(2, "GDSQlite Error: Did not find required key \"type\" in the supplied json-file")
            return false;
        }
        if (temp_dict["type"] == String("table"))
        {
            new_object.type = TABLE;

            /* Add the optional base64_columns if defined! */
            new_object.base64_columns = get_with_default(temp_dict, "base64_columns", Array());

            if (!temp_dict.has("row_array"))
            {
                /* Did not find the necessary key! */
                GODOT_LOG(2, "GDSQlite Error: Did not find required key \"row_array\" in the supplied json-file")
                return false;
            }
            else if (Variant(temp_dict["row_array"]).get_type() != Variant::ARRAY)
            {
                GODOT_LOG(2, "GDSQlite Error: The value of the key \"row_array\" should consist of an array of rows")
                return false;
            }
            new_object.row_array = temp_dict["row_array"];

        }
        else if (temp_dict["type"] == String("trigger"))
        {
            new_object.type = TRIGGER;
        }
        else
        {
            /* Did not find the necessary key! */
            GODOT_LOG(2, "GDSQlite Error: The value of key \"type\" is restricted to either \"table\" or \"trigger\"")
            return false;
        }

        /* Add the table or trigger to the new objects vector */
        objects_to_import.insert(objects_to_import.end(), new_object);
    }
    return true;
}

Dictionary SQLite::deep_copy(Dictionary p_dict)
{
    Dictionary copy_dict;
    Array keys = p_dict.keys();
    int number_of_keys = keys.size();
    for (int i = 0; i <= number_of_keys - 1; i++)
    {
        copy_dict[keys[i]] = p_dict[keys[i]];
    }
    return copy_dict;
}

Variant SQLite::get_with_default(Dictionary p_dict, String p_key, Variant p_default)
{
    if (p_dict.has(p_key))
    {
        return p_dict[p_key];
    }
    else
    {
        return p_default;
    }
}
