#include "gdsqlite.h"
#include "../sqlite/sqlite3.h"
#include <stdio.h>

using namespace godot;

void SQLite::_register_methods()
{

    register_method("open_db", &SQLite::open_db);
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
    String ending = String(".db");
    if (!path.ends_with(ending))
    {
        path += ending;
    }
    /* Find the real path */
    path = ProjectSettings::get_singleton()->globalize_path(path.strip_edges());

    /* C++ Garbage collector deletes CharString object when not assigned explicitely */
    const CharString dummy_path = path.utf8();
    const char *char_path = dummy_path.get_data();

    /* Try to open the database */
    rc = sqlite3_open(char_path, &db);

    if (rc)
    {
        Godot::print("Can't open database: " + String(sqlite3_errmsg(db)));
        return (false);
    }
    else
    {
        Godot::print("Opened database successfully (" + path + ")");
    }
    return (true);
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
    /* C++ Garbage collector deletes CharString object when not assigned explicitely */
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

bool SQLite::create_table(String p_table, Dictionary p_table_dict)
{

    String query_string;
    /* Create SQL statement */
    query_string = "CREATE TABLE IF NOT EXISTS " + p_table + " (";

    Dictionary column_dict;
    Array columns = p_table_dict.keys();
    int number_of_columns = columns.size();
    for (int i = 0; i <= number_of_columns - 1; i++)
    {
        column_dict = p_table_dict[columns[i]];
        if (!column_dict.has("data_type"))
        {
            Godot::print("The field 'data_type' is a required part of the dictionary");
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

bool SQLite::drop_table(String p_table)
{

    String query_string;
    /* Create SQL statement */
    query_string = "DROP TABLE " + p_table + ";";

    return query(query_string);
}

bool SQLite::insert_row(String p_table, Dictionary p_row_dict)
{

    String query_string, key_string, value_string;
    Array keys = p_row_dict.keys();
    Array values = p_row_dict.values();

    query_string = "INSERT INTO " + p_table;

    int number_of_keys = p_row_dict.size();

    key_string += (const String &)keys[0];
    value_string += (const String &)values[0];
    for (int i = 1; i <= number_of_keys - 1; i++)
    {
        key_string += "," + (const String &)keys[i];
        if (values[i].get_type() == Variant::STRING)
        {
            value_string += ",'" + (const String &)values[i] + "'";
        }
        else
        {
            value_string += "," + (const String &)values[i];
        }
    }
    query_string += " (" + key_string + ") VALUES (" + value_string + ");";

    return query(query_string);
}

bool SQLite::insert_rows(String p_table, Array p_row_array)
{
    int number_of_rows = p_row_array.size();
    for (int i = 1; i <= number_of_rows - 1; i++)
    {
        if (p_row_array[i].get_type() != Variant::DICTIONARY)
        {
            Godot::print("All elements of the Array should be of type Dictionary");
            return false;
        }
        if (!insert_row(p_table, p_row_array[i]))
        {
            return false;
        }
    }
    return true;
}

Array SQLite::select_rows(String p_table, String p_conditions, Array p_columns_array)
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
    query_string += " from " + p_table;
    if (!p_conditions.empty())
    {
        query_string += " WHERE " + p_conditions;
    }
    query_string += ";";

    query(query_string);
    return query_result;
}

bool SQLite::update_rows(String p_table, String p_conditions, Dictionary p_updated_row_dict)
{

    String query_string;
    int number_of_keys = p_updated_row_dict.size();
    Array keys = p_updated_row_dict.keys();
    Array values = p_updated_row_dict.values();

    /* Create SQL statement */
    query_string = "UPDATE " + p_table + " SET ";

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

bool SQLite::delete_rows(String p_table, String p_conditions)
{
    String query_string;

    /* Create SQL statement */
    query_string = "DELETE FROM " + p_table;
    /* If it's empty or * everything is to be deleted */
    if (!p_conditions.empty() && (p_conditions != (const String &)"*") )
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
