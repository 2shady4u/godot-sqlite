#include "gdsqlite.h"

using namespace godot;

void SQLite::_register_methods()
{

    register_method("open_db", &SQLite::open_db);
    register_method("import_from_json", &SQLite::import_from_json);
    register_method("export_to_json", &SQLite::export_to_json);
    register_method("close_db", &SQLite::close_db);
    register_method("query", &SQLite::query);

    register_method("create_table", &SQLite::create_table);
    register_method("drop_table", &SQLite::drop_table);

    register_method("insert_row", &SQLite::insert_row);
    register_method("insert_rows", &SQLite::insert_rows);

    register_method("select_rows", &SQLite::select_rows);
    register_method("update_rows", &SQLite::update_rows);
    register_method("delete_rows", &SQLite::delete_rows);

    register_property<SQLite, String>("path", &SQLite::set_path, &SQLite::get_path, "default");
    register_property<SQLite, bool>("verbose_mode", &SQLite::verbose_mode, false);
    register_property<SQLite, Array>("query_result", &SQLite::query_result, Array());
}

SQLite::SQLite()
{
    db = nullptr;
    query_result = Array();
}

SQLite::~SQLite()
{
    close_db();
}

void SQLite::_init()
{
    path = String("default");
    verbose_mode = false;
}

bool SQLite::open_db()
{

    char *zErrMsg = 0;
    int rc;
    /* Add .db to the path String if not present */
    String ending = String(".db");
    if (!path.ends_with(ending))
    {
        path += ending;
    }
    /* Find the real path */
    path = ProjectSettings::get_singleton()->globalize_path(path.strip_edges());

    /* CharString object goes out-of-scope when not assigned explicitely */
    const CharString dummy_path = path.utf8();
    const char *char_path = dummy_path.get_data();

    /* Try to open the database */
    rc = sqlite3_open(char_path, &db);

    if (rc)
    {
        Godot::print("GDSQLite Error: Can't open database: " + String(sqlite3_errmsg(db)));
        return false;
    }
    else
    {
        Godot::print("Opened database successfully (" + path + ")");
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

    /* CharString object goes out-of-scope when not assigned explicitely */
    const CharString dummy_path = import_path.utf8();
    const char *char_path = dummy_path.get_data();

    /* Open the json-file and parse its contents into a nlohmann json object */
    std::ifstream ifs(char_path);
    if (ifs.fail())
    {
        Godot::print("GDSQLite Error: Can't open specified json-file, file does not exist or is locked");
        return false;
    }
    /* Attempt to open the json and, if unsuccessful, throw a parse error specifying the erroneous line */
    json import_json;
    try
    {
        import_json = json::parse(ifs);
    }
    catch (json::parse_error &e)
    {
        /* Throw a parsing error */
        Godot::print("GDSQLite Error: " + String(e.what()));
        return false;
    }
    ifs.close();

    /* Check if the database is open and, if not, attempt to open it */
    if (db == nullptr)
    {
        /* Open the database using the open_db method declared above */
        open_db();
    }
    /* Find all tables that are present in this database */
    query(String("SELECT name FROM sqlite_master WHERE type = 'table';"));
    Array old_table_names = Array();
    int old_number_of_tables = query_result.size();
    for (int i = 0; i <= old_number_of_tables - 1; i++)
    {
        Dictionary table_dict = query_result[i];
        old_table_names.append(String(table_dict["name"]));
    }

    std::vector<table_struct> tables_to_import;
    /* Validate the json structure and populate the tables_to_import vector */
    if (!validate_json(import_json, tables_to_import))
    {
        return false;
    }

    /* Drop all old tables present in the database */
    for (int i = 0; i <= old_number_of_tables - 1; i++)
    {
        drop_table(old_table_names[i]);
    }
    /* Add all new tables and fill them up with all the rows */
    for (auto table : tables_to_import)
    {
        query(table.sql);
        insert_rows(table.name, table.row_array);
    }
    return true;
}


bool SQLite::export_to_json(String export_path) 
{ 
    /* Get all names and sql templates for all tables present in the database */ 
    query(String("SELECT name,sql FROM sqlite_master WHERE type = 'table';")); 
    int number_of_tables = query_result.size(); 
    Array database_dict; 
    for (int i = 0; i <= number_of_tables - 1; i++) 
    { 
        /* Deep copy of the Dictionary is required as Dictionaries are passed with reference */ 
        /* Without doing this, future queries would overwrite the table information */ 
        database_dict.append(deep_copy(query_result[i])); 
    } 
 
    /* Add .json to the import_path String if not present */
    String ending = String(".json"); 
    if (!export_path.ends_with(ending)) 
    { 
        export_path += ending; 
    }
    /* Find the real path */
    export_path = ProjectSettings::get_singleton()->globalize_path(export_path.strip_edges());
 
    /* CharString object goes out-of-scope when not assigned explicitely */ 
    const CharString dummy_path = export_path.utf8(); 
    const char *char_path = dummy_path.get_data(); 
 
    std::ofstream ofs(char_path, std::ios::trunc); 
    if (ofs.fail())
    {
        Godot::print("GDSQLite Error: Can't open specified json-file, file does not exist or is locked");
        return false;
    }

    /* Construct a Dictionary for each table, convert it to JSON and write it to file */ 
    ofs << "["; 
    for (int i = 0; i <= number_of_tables - 1; i++) 
    { 
        Dictionary table_dict = database_dict[i]; 
        String json_string, query_string; 
 
        query_string = "SELECT * FROM " + String(table_dict["name"]) + ";"; 
        query(query_string); 
        table_dict["row_array"] = query_result; 
 
        json_string = table_dict.to_json(); 
        /* CharString object goes out-of-scope when not assigned explicitely */ 
        const CharString dummy_string = json_string.utf8(); 
        const char *json_char = dummy_string.get_data(); 
        ofs << json_char; 
        if (i != number_of_tables - 1) 
        { 
            ofs << ","; 
        } 
    } 
    ofs << "]"; 
    ofs.close(); 
    return true; 
} 

void SQLite::close_db()
{
    if (db)
    {
        // Cannot close database!
        if (sqlite3_close_v2(db) != SQLITE_OK)
        {
            Godot::print("Cannot close database!");
        }
        else
        {
            db = nullptr;
            Godot::print("Closed database (" + path + ")");
        }
    }
}

static int callback(void *closure, int argc, char **argv, char **azColName)
{

    Dictionary column_dict;
    /* Get a reference to the instanced object */
    SQLite *obj = static_cast<SQLite *>(closure);
    sqlite3_stmt *stmt = sqlite3_next_stmt(obj->db, NULL);
    Variant column_value;

    /* Loop over all columns and add them to the Dictionary */
    for (int i = 0; i < argc; i++)
    {
        /* Check the column type and do correct casting */
        switch (sqlite3_column_type(stmt, i))
        {
        case SQLITE_INTEGER:
            column_value = Variant(sqlite3_column_int(stmt, i));
            break;

        case SQLITE_FLOAT:
            column_value = Variant(sqlite3_column_double(stmt, i));
            break;

        case SQLITE_TEXT:
            column_value = Variant((char *)sqlite3_column_text(stmt, i));
            break;

        default:
            column_value = Variant((char *)sqlite3_column_text(stmt, i));
            break;
        }
        column_dict[String(azColName[i])] = column_value;
    }
    /* Add result to query_result Array */
    obj->query_result.append(column_dict);

    return 0;
}

bool SQLite::query(String p_query)
{
    char *zErrMsg = 0;
    int rc;
    const char *sql;

    if (verbose_mode)
    {
        Godot::print(p_query);
    }
    /* CharString object goes out-of-scope when not assigned explicitely */
    const CharString dummy = p_query.utf8();
    sql = dummy.get_data();

    /* Clear the previous query results */
    query_result.clear();

    /* Execute SQL statement */
    rc = sqlite3_exec(db, sql, callback, (void *)this, &zErrMsg);

    if (rc != SQLITE_OK)
    {
        Godot::print(" --> SQL error: " + String(zErrMsg));
        sqlite3_free(zErrMsg);
        return false;
    }
    else if (verbose_mode)
    {
        Godot::print(" --> Query succeeded");
    }
    return true;
}

bool SQLite::create_table(String p_name, Dictionary p_table_dict)
{

    String query_string;
    /* Create SQL statement */
    query_string = "CREATE TABLE IF NOT EXISTS " + p_name + " (";

    Dictionary column_dict;
    Array columns = p_table_dict.keys();
    int number_of_columns = columns.size();
    for (int i = 0; i <= number_of_columns - 1; i++)
    {
        column_dict = p_table_dict[columns[i]];
        if (!column_dict.has("data_type"))
        {
            Godot::print("GDSQLite Error: The field 'data_type' is a required part of the table dictionary");
            return false;
        }
        query_string += (const String &)columns[i] + " " + column_dict["data_type"];

        if (column_dict.has("primary_key"))
        {
            if (column_dict["primary_key"])
            {
                query_string += String(" PRIMARY KEY");
            }
        }
        if (column_dict.has("not_null"))
        {
            if (column_dict["not_null"])
            {
                query_string += String(" NOT NULL");
            }
        }

        if (i != number_of_columns - 1)
        {
            query_string += ",";
        }
    }
    query_string += ");";

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
    Array values = p_row_dict.values();

    query_string = "INSERT INTO " + p_name;

    int number_of_keys = p_row_dict.size();

    for (int i = 0; i <= number_of_keys - 1; i++)
    {
        key_string += (const String &)keys[i];
        if (values[i].get_type() == Variant::STRING)
        {
            value_string += "'" + (const String &)values[i] + "'";
        }
        else
        {
            value_string += (const String &)values[i];
        }
        if (i!=number_of_keys-1)
        {
            key_string += ",";
            value_string += ",";
        }
    }
    query_string += " (" + key_string + ") VALUES (" + value_string + ");";

    return query(query_string);
}

bool SQLite::insert_rows(String p_name, Array p_row_array)
{
    int number_of_rows = p_row_array.size();
    for (int i = 0; i <= number_of_rows - 1; i++)
    {
        if (p_row_array[i].get_type() != Variant::DICTIONARY)
        {
            Godot::print("All elements of the Array should be of type Dictionary");
            return false;
        }
        if (!insert_row(p_name, p_row_array[i]))
        {
            return false;
        }
    }
    return true;
}

Array SQLite::select_rows(String p_name, String p_conditions, Array p_columns_array)
{

    String query_string;
    int number_of_columns = p_columns_array.size();

    /* Create SQL statement */
    query_string = "SELECT ";
    for (int i = 0; i <= number_of_columns - 1; i++)
    {
        if (p_columns_array[i].get_type() != Variant::STRING)
        {
            Godot::print("All elements of the Array should be of type String");
            return query_result;
        }
        query_string += (const String &)p_columns_array[i];

        if (i != number_of_columns - 1)
        {
            query_string += ",";
        }
    }
    query_string += " from " + p_name;
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
    int number_of_keys = p_updated_row_dict.size();
    Array keys = p_updated_row_dict.keys();
    Array values = p_updated_row_dict.values();

    /* Create SQL statement */
    query_string = "UPDATE " + p_name + " SET ";

    for (int i = 0; i <= number_of_keys - 1; i++)
    {
        query_string += (const String &)keys[i] + "=";
        if (values[i].get_type() == Variant::STRING)
        {
            query_string += "'" + (const String &)values[i] + "'";
        }
        else
        {
            query_string += (const String &)values[i];
        }

        if (i != number_of_keys - 1)
        {
            query_string += ",";
        }
    }
    query_string += " WHERE " + p_conditions + ";";

    return query(query_string);
}

bool SQLite::delete_rows(String p_name, String p_conditions)
{
    String query_string;

    /* Create SQL statement */
    query_string = "DELETE FROM " + p_name;
    /* If it's empty or * everything is to be deleted */
    if (!p_conditions.empty() && (p_conditions != (const String &)"*"))
    {
        query_string += " WHERE " + p_conditions;
    }
    query_string += ";";

    return query(query_string);
}

void SQLite::set_path(String p_path)
{
    path = p_path;
}

String SQLite::get_path()
{
    return path;
}

bool SQLite::validate_json(json import_json, std::vector<table_struct> &tables_to_import)
{
    if (!import_json.is_array())
    {
        Godot::print("GDSQlite Error: Supplied json-file should consist of an array of tables");
        return false;
    }

    /* Start going through all the tables and checking their validity */
    auto number_of_tables = import_json.size();
    for (decltype(number_of_tables) i = 0; i <= number_of_tables - 1; i++)
    {
        /* Create a new table_struct */
        table_struct new_table;

        /* Get the name of the table */
        auto it_name = import_json[i].find("name");
        if (it_name == import_json[i].end())
        {
            /* Did not find the necessary key! */
            Godot::print("GDSQlite Error: Did not find required key \"name\" in the supplied json-file");
            return false;
        }
        std::string string_name = *it_name;
        const char *char_name = string_name.data();
        new_table.name = String(char_name);

        /* Extract the sql template for generating the table */
        auto it_sql = import_json[i].find("sql");
        if (it_sql == import_json[i].end())
        {
            /* Did not find the necessary key! */
            Godot::print("GDSQlite Error: Did not find required key \"sql\" in the supplied json-file");
            return false;
        }
        std::string string_sql = *it_sql;
        const char *char_sql = string_sql.data();
        new_table.sql = String(char_sql);

        auto it_rows = import_json[i].find("row_array");
        if (it_rows == import_json[i].end())
        {
            /* Did not find the necessary key! */
            Godot::print("GDSQlite Error: Did not find required key \"row_array\" in the supplied json-file");
            return false;
        }
        if (!import_json[i]["row_array"].is_array())
        {
            Godot::print("GDSQlite Error: The value of the key \"row_array\" should consist of an array of rows");
            return false;
        }
        /* Get the rows and parse them as an array of dictionaries */
        Array row_array = Array();
        auto number_of_rows = import_json[i]["row_array"].size();
        for (decltype(number_of_rows) j = 0; j <= number_of_rows - 1; j++)
        {
            Dictionary row_dict = parse_json(import_json[i]["row_array"][j]);
            row_array.append(row_dict);
        }
        new_table.row_array = row_array;

        /* Add the table to the new tables vector */
        tables_to_import.insert(tables_to_import.end(), new_table);
    }
    return true;
}

Dictionary SQLite::deep_copy(Dictionary original_dict)
{
    Dictionary copy_dict;
    Array keys = original_dict.keys();
    int number_of_keys = keys.size();
    for (int i = 0; i <= number_of_keys - 1; i++)
    {
        copy_dict[keys[i]] = original_dict[keys[i]];
    }
    return copy_dict;
}

Dictionary SQLite::parse_json(json json_to_parse)
{
    Dictionary parsed_dict = Dictionary();
    for (auto &item : json_to_parse.items())
    {
        /* Do lots of conversion work... */
        auto string_key = item.key();
        const char *char_key = string_key.data();
        /* Add the key and, the correctly casted, value to the Dictionary */
        if (item.value().is_object())
        {
            /* The value is itself a dictionary and has to be correctly casted as well */
            /* recursive call might be overkill... */
            Dictionary dict_value = parse_json(item.value());
            parsed_dict[String(char_key)] = dict_value;
        }
        else
        {
            auto string_value = item.value().dump();
            const char *char_value = string_value.data();
            /* Remove the unnecessary "" coming out of the dump() */
            parsed_dict[String(char_key)] = String(char_value).replace(String("\""), String(""));
        }
    }
    return parsed_dict;
}