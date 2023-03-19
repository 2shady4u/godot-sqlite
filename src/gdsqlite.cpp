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

    register_method("get_autocommit", &SQLite::get_autocommit);

    register_property<SQLite, int>("last_insert_rowid", &SQLite::set_last_insert_rowid, &SQLite::get_last_insert_rowid, 0);
    register_property<SQLite, int>("verbosity_level", &SQLite::set_verbosity_level, &SQLite::get_verbosity_level, VerbosityLevel::NORMAL);

    register_property<SQLite, bool>("verbose_mode", &SQLite::set_verbose_mode, &SQLite::get_verbose_mode, false);
    register_property<SQLite, bool>("foreign_keys", &SQLite::foreign_keys, false);
    register_property<SQLite, bool>("read_only", &SQLite::read_only, false);

    register_property<SQLite, String>("path", &SQLite::path, "default");
    register_property<SQLite, String>("error_message", &SQLite::error_message, "");
    register_property<SQLite, String>("default_extension", &SQLite::default_extension, "db");

    register_property<SQLite, Array>("query_result", &SQLite::set_query_result, &SQLite::get_query_result, Array());
    register_property<SQLite, Array>("query_result_by_reference", &SQLite::set_query_result, &SQLite::get_query_result_by_reference, Array());
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
    verbosity_level = VerbosityLevel::NORMAL;
    foreign_keys = false;
    read_only = false;
    path = String("default");
    default_extension = String("db");
}

bool SQLite::open_db()
{
    char *zErrMsg = 0;
    int rc;
    if (path.find(":memory:") == -1)
    {
        /* Add the default_extension to the database path if no extension is present */
        /* Skip if the default_extension is an empty string to allow for paths without extension */
        if (path.get_extension().empty() && !default_extension.empty())
        {
            String ending = String(".") + default_extension;
            path += ending;
        }

        if (!read_only)
        {
            /* Find the real path */
            path = ProjectSettings::get_singleton()->globalize_path(path.strip_edges());
        }

        /* This part does weird things on Android & on export! Leave it out for now! */
        ///* Make the necessary empty directories if they do not exist yet */
        // Ref<Directory> dir = Directory::_new();
        // PoolStringArray split_array = path.split("/", false);
        ///* Remove the database filename */
        // split_array.remove(split_array.size()-1);
        // String path_without_file = "";
        // for (int i = 0; i < split_array.size(); i++) {
        //     path_without_file += split_array[i] + "/";
        // }
        // Error error = dir->make_dir_recursive(path_without_file);
        // if (error != Error::OK){
        //     GODOT_LOG(2, "GDSQLite Error: Can't make necessary folders for path (ERROR = "
        //     + String(std::to_string(static_cast<int>(error)).c_str())
        //     + ")")
        //     return false;
        // }
    }

    /* NOTE: Memory allocated by the alloc_c_string()-method needs to be freed manually! */
    const char *char_path = path.alloc_c_string();
    /* Try to open the database */
    if (read_only)
    {
        if (path.find(":memory:") == -1)
        {
            sqlite3_vfs_register(gdsqlite_vfs(), 0);
            rc = sqlite3_open_v2(char_path, &db, SQLITE_OPEN_READONLY, "godot");
        }
        else
        {
            GODOT_LOG(2, "GDSQLite Error: Opening in-memory databases in read-only mode is currently not supported!")
            api->godot_free((void *)char_path);
            return false;
        }
    }
    else
    {
#ifdef __EMSCRIPTEN__
        /* In the case of the web build, we'll have to use the custom VFS such that the file system (IndexedDB) gets synced correctly */
        /* Syncing the file system doesn't happen automatically nor is it, at the time of writing, possible to manually trigger a synchronization event */
        /* The custom VFS uses Godot's File class internally which syncs the file system whenever the file gets closed */
        sqlite3_vfs_register(gdsqlite_vfs(), 0);
        rc = sqlite3_open_v2(char_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, "godot");
#else
        rc = sqlite3_open_v2(char_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL);
#endif
        /* The first two flags are default flags with behaviour that is identical to: `rc = sqlite3_open(char_path, &db);`*/
        /* The SQLITE_OPEN_URI flag is solely useful when using shared in-memory databases (shared cache), but it is safe to include in most situations */
        /* As found in the SQLite documentation: (https://www.sqlite.org/uri.html)*/

        /*     Since SQLite always interprets any filename that does not begin with "file:" as an ordinary filename regardless of the URI */
        /*     setting, and because it is very unusual to have an actual file begin with "file:", it is safe for most applications to enable URI */
        /*     processing even if URI filenames are not currently being used. */

        /* In-memory databases with shard cache can be opened by setting the path-variable to `file::memory:?cache=shared` */
        /* More information can be found here: https://www.sqlite.org/inmemorydb.html */
    }

    if (rc != SQLITE_OK)
    {
        GODOT_LOG(2, "GDSQLite Error: Can't open database: " + String(sqlite3_errmsg(db)))
        api->godot_free((void *)char_path);
        return false;
    }
    else if (verbosity_level > VerbosityLevel::QUIET)
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
            api->godot_free((void *)char_path);
            return false;
        }
    }

    api->godot_free((void *)char_path);
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
            if (verbosity_level > VerbosityLevel::QUIET)
            {
                GODOT_LOG(0, "Closed database (" + path + ")")
            }
        }
    }
}

bool SQLite::query(String p_query)
{
    return query_with_bindings(p_query, Array());
}

bool SQLite::query_with_bindings(String p_query, Array param_bindings)
{
    const char *zErrMsg, *sql, *pzTail;
    int rc;

    if (verbosity_level > VerbosityLevel::NORMAL)
    {
        GODOT_LOG(0, p_query)
    }
    sql = p_query.alloc_c_string();

    /* Clear the previous query results */
    query_result.clear();

    sqlite3_stmt *stmt;
    /* Prepare an SQL statement */
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, &pzTail);
    zErrMsg = sqlite3_errmsg(db);
    error_message = String(zErrMsg);
    if (rc != SQLITE_OK)
    {
        GODOT_LOG(2, " --> SQL error: " + error_message)
        sqlite3_finalize(stmt);
        api->godot_free((void *)sql);
        return false;
    }

    /* Check if the param_bindings size exceeds the required parameter count */
    int parameter_count = sqlite3_bind_parameter_count(stmt);
    if (param_bindings.size() < parameter_count)
    {
        GODOT_LOG(2, "GDSQLite Error: Insufficient number of parameters to satisfy required number of bindings in statement!")
        sqlite3_finalize(stmt);
        api->godot_free((void *)sql);
        return false;
    }

    /* Bind any given parameters to the prepared statement */
    for (int i = 0; i < parameter_count; i++)
    {
        Variant binding_value = param_bindings.pop_front();
        switch (binding_value.get_type())
        {
        case Variant::NIL:
            sqlite3_bind_null(stmt, i + 1);
            break;

        case Variant::BOOL:
        case Variant::INT:
            sqlite3_bind_int64(stmt, i + 1, int64_t(binding_value));
            break;

        case Variant::REAL:
            sqlite3_bind_double(stmt, i + 1, binding_value);
            break;

        case Variant::STRING:
        {
            const char *char_binding = (binding_value.operator String()).alloc_c_string();
            sqlite3_bind_text(stmt, i + 1, char_binding, -1, SQLITE_TRANSIENT);
            api->godot_free((void *)char_binding);
            break;
        }

        case Variant::POOL_BYTE_ARRAY:
        {
            PoolByteArray binding = ((const PoolByteArray &)binding_value);
            PoolByteArray::Read r = binding.read();
            sqlite3_bind_blob64(stmt, i + 1, r.ptr(), binding.size(), SQLITE_TRANSIENT);
            break;
        }

        default:
            GODOT_LOG(2, "GDSQLite Error: Binding a parameter of type " + String(std::to_string(binding_value.get_type()).c_str()) + " (TYPE_*) is not supported!")
            sqlite3_finalize(stmt);
            api->godot_free((void *)sql);
            return false;
        }
    }

    if (verbosity_level > VerbosityLevel::NORMAL)
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
        api->godot_free((void *)sql);
        return false;
    }
    else if (verbosity_level > VerbosityLevel::NORMAL)
    {
        GODOT_LOG(0, " --> Query succeeded")
    }

    /* Figure out if there's a subsequent statement which needs execution */
    String sTail = String(pzTail).strip_edges();
    if (!sTail.empty())
    {
        api->godot_free((void *)sql);
        return query_with_bindings(sTail, param_bindings);
    }

    if (!param_bindings.empty())
    {
        GODOT_LOG(1, "GDSQLite Warning: Provided number of bindings exceeded the required number in statement! (" + String(std::to_string(param_bindings.size()).c_str()) + " unused parameter(s))")
    }

    api->godot_free((void *)sql);
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
        /* Unique check */
        if (get_with_default(column_dict, "unique", false))
        {
            query_string += String(" UNIQUE");
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
            /* Don't forget to close the transaction! */
            /* Maybe we should do a rollback instead? */
            query("END TRANSACTION;");
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
    /* Return the duplicated result */
    return get_query_result();
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
    {
        const char *char_output = (output.operator String()).alloc_c_string();
        sqlite3_result_text(context, char_output, -1, SQLITE_TRANSIENT);
        api->godot_free((void *)char_output);
        break;
    }

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
        api->godot_free((void *)zFunctionName);
        return false;
    }
    else if (verbosity_level > VerbosityLevel::NORMAL)
    {
        GODOT_LOG(0, "Succesfully added function \"" + p_name + "\" to function registry")
    }
    api->godot_free((void *)zFunctionName);
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
        api->godot_free((void *)char_path);
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
        api->godot_free((void *)char_path);
        return false;
    }
    Array database_array = result->get_result();
    std::vector<object_struct> objects_to_import;
    /* Validate the json structure and populate the tables_to_import vector */
    if (!validate_json(database_array, objects_to_import))
    {
        api->godot_free((void *)char_path);
        return false;
    }

    /* Check if the database is open and, if not, attempt to open it */
    if (db == nullptr)
    {
        /* Open the database using the open_db method */
        if (!open_db())
        {
            api->godot_free((void *)char_path);
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
                    String encoded_string = ((const String &)row[key]);
                    PoolByteArray arr = Marshalls::get_singleton()->base64_to_raw(encoded_string);
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
                api->godot_free((void *)char_path);
                return false;
            }
            if (!insert_row(object.name, object.row_array[i]))
            {
                /* Don't forget to close the transaction! */
                /* Stop the error_message from being overwritten! */
                String previous_error_message = error_message;
                query("END TRANSACTION;");
                error_message = previous_error_message;
                api->godot_free((void *)char_path);
                return false;
            }
        }
    }
    query("END TRANSACTION;");
    api->godot_free((void *)char_path);
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

        if (object_dict["type"] == String("table"))
        {
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
                        String encoded_string = Marshalls::get_singleton()->raw_to_base64(arr);

                        row.erase(key);
                        row[key] = encoded_string;
                    }
                }

                if (!base64_columns.empty())
                {
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
        api->godot_free((void *)char_path);
        return false;
    }
    String json_string = JSON::get_singleton()->print(database_array, "\t");
    const char *json_char = json_string.alloc_c_string();
    ofs << json_char;
    api->godot_free((void *)json_char);
    ofs.close();

    api->godot_free((void *)char_path);
    return true;
}

void SQLite::set_last_insert_rowid(int p_last_insert_rowid)
{
    if (db)
    {
        sqlite3_set_last_insert_rowid(db, p_last_insert_rowid);
    }
}

int SQLite::get_last_insert_rowid()
{
    if (db)
    {
        return sqlite3_last_insert_rowid(db);
    }
    /* Return the default value */
    return 0;
}

void SQLite::set_verbosity_level(int p_verbosity_level)
{
    verbosity_level = p_verbosity_level;
}

int SQLite::get_verbosity_level()
{
    return verbosity_level;
}

void SQLite::set_verbose_mode(bool p_verbose_mode)
{
    if (p_verbose_mode)
    {
        set_verbosity_level(VerbosityLevel::VERBOSE);
    }
    else 
    {
        set_verbosity_level(VerbosityLevel::QUIET);
    }
}

bool SQLite::get_verbose_mode()
{
    return verbosity_level > VerbosityLevel::QUIET;
}

void SQLite::set_query_result(Array p_query_result)
{
    query_result = p_query_result;
}

Array SQLite::get_query_result()
{
    return query_result.duplicate(true);
}

Array SQLite::get_query_result_by_reference()
{
    return query_result;
}

int SQLite::get_autocommit()
{
    if (db)
    {
        return sqlite3_get_autocommit(db);
    }
    /* Return the default value */
    return 1; // A non-zero value indicates the autocommit is on!
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
