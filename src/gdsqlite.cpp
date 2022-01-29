#include "gdsqlite.h"

#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/project_settings.hpp>

using namespace godot;

void SQLite::_bind_methods()
{
    // Methods.
    ClassDB::bind_method(D_METHOD("open_db"), &SQLite::open_db);
    ClassDB::bind_method(D_METHOD("close_db"), &SQLite::close_db);
    ClassDB::bind_method(D_METHOD("query"), &SQLite::query);
    ClassDB::bind_method(D_METHOD("query_with_bindings"), &SQLite::query_with_bindings);

    ClassDB::bind_method(D_METHOD("create_table"), &SQLite::create_table);
    ClassDB::bind_method(D_METHOD("drop_table"), &SQLite::drop_table);

    ClassDB::bind_method(D_METHOD("insert_row"), &SQLite::insert_row);
    ClassDB::bind_method(D_METHOD("insert_rows"), &SQLite::insert_rows);

    ClassDB::bind_method(D_METHOD("select_rows"), &SQLite::select_rows);
    ClassDB::bind_method(D_METHOD("update_rows"), &SQLite::update_rows);
    ClassDB::bind_method(D_METHOD("delete_rows"), &SQLite::delete_rows);

    // Properties.
    ClassDB::bind_method(D_METHOD("set_verbose_mode", "verbose_mode"), &SQLite::set_verbose_mode);
    ClassDB::bind_method(D_METHOD("get_verbose_mode"), &SQLite::get_verbose_mode);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "verbose_mode"), "set_verbose_mode", "get_verbose_mode");

    ClassDB::bind_method(D_METHOD("set_foreign_keys", "foreign_keys"), &SQLite::set_foreign_keys);
    ClassDB::bind_method(D_METHOD("get_foreign_keys"), &SQLite::get_foreign_keys);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "foreign_keys"), "set_foreign_keys", "get_foreign_keys");

    ClassDB::bind_method(D_METHOD("set_read_only", "read_only"), &SQLite::set_read_only);
    ClassDB::bind_method(D_METHOD("get_read_only"), &SQLite::get_read_only);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "read_only"), "set_read_only", "get_read_only");

    ClassDB::bind_method(D_METHOD("set_path", "path"), &SQLite::set_path);
    ClassDB::bind_method(D_METHOD("get_path"), &SQLite::get_path);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "path"), "set_path", "get_path");

    ClassDB::bind_method(D_METHOD("set_error_message", "error_message"), &SQLite::set_error_message);
    ClassDB::bind_method(D_METHOD("get_error_message"), &SQLite::get_error_message);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "error_message"), "set_error_message", "get_error_message");

    ClassDB::bind_method(D_METHOD("set_default_extension", "default_extension"), &SQLite::set_default_extension);
    ClassDB::bind_method(D_METHOD("get_default_extension"), &SQLite::get_default_extension);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "default_extension"), "set_default_extension", "get_default_extension");

    ClassDB::bind_method(D_METHOD("set_query_result", "query_result"), &SQLite::set_query_result);
    ClassDB::bind_method(D_METHOD("get_query_result"), &SQLite::get_query_result);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "query_result"), "set_query_result", "get_query_result");
}

SQLite::SQLite()
{
    UtilityFunctions::print("constructor");
    db = nullptr;
}

SQLite::~SQLite()
{
    UtilityFunctions::print("destructor");
    /* Automatically close the open database connection */
    close_db();
}

bool SQLite::open_db()
{
    char *zErrMsg = 0;
    int rc;
    if (path != ":memory:")
    {
        /* Add the default_extension to the database path if no extension is present */
        /* Skip if the default_extension is an empty string to allow for paths without extension */
        if (path.get_extension().is_empty() && !default_extension.is_empty())
        {
            String ending = String(".") + default_extension;
            // TODO: Switch back to += once the API gets updated
            path = path + ending;
        }

        if (!read_only)
        {
            /* Find the real path */
            path = ProjectSettings::get_singleton()->globalize_path(path.strip_edges());
        }
    }

    // TODO: Switch back to the `alloc_c_string()`-method once the API gets updated
    const CharString dummy_path = path.utf8();
    const char *char_path = dummy_path.get_data();
    //const char *char_path = path.alloc_c_string();
    /* Try to open the database */
    if (read_only)
    {
        if (path != ":memory:")
        {
            // TODO: Re-enable VFS at some point
            UtilityFunctions::printerr("GDSQLite Error: The Virtual File System component is not yet supported for GDExtension");
            //sqlite3_vfs_register(gdsqlite_vfs(), 0);
            //rc = sqlite3_open_v2(char_path, &db, SQLITE_OPEN_READONLY, "godot");
            return false;
        }
        else
        {
            UtilityFunctions::printerr("GDSQLite Error: Opening in-memory databases in read-only mode is currently not supported!");
            return false;
        }
    }
    else
    {
        rc = sqlite3_open_v2(char_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL);
        /* Identical to: `rc = sqlite3_open(char_path, &db);`*/
    }

    if (rc != SQLITE_OK)
    {
        UtilityFunctions::printerr(2, "GDSQLite Error: Can't open database: " + String(sqlite3_errmsg(db)));
        return false;
    }
    else
    {
        UtilityFunctions::print("Opened database successfully (" + path + ")");
    }

    /* Try to enable foreign keys. */
    if (foreign_keys)
    {
        rc = sqlite3_exec(db, "PRAGMA foreign_keys=on;", NULL, NULL, &zErrMsg);
        if (rc != SQLITE_OK)
        {
            UtilityFunctions::printerr("GDSQLite Error: Can't enable foreign keys: " + String(zErrMsg));
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
            UtilityFunctions::printerr("GDSQLite Error: Can't close database!");
        }
        else
        {
            db = nullptr;
            UtilityFunctions::print("Closed database (" + path + ")");
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
        UtilityFunctions::print(p_query);
    }
    // TODO: Switch back to the `alloc_c_string()`-method once the API gets updated
    const CharString dummy_query = p_query.utf8();
    sql = dummy_query.get_data();
    //sql = p_query.alloc_c_string();

    /* Clear the previous query results */
    query_result.clear();

    sqlite3_stmt *stmt;
    /* Prepare an SQL statement */
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, NULL);
    zErrMsg = sqlite3_errmsg(db);
    error_message = String(zErrMsg);
    if (rc != SQLITE_OK)
    {
        UtilityFunctions::printerr(" --> SQL error: " + error_message);
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

        case Variant::FLOAT:
            sqlite3_bind_double(stmt, i + 1, param_bindings[i]);
            break;

        case Variant::STRING:
            // TODO: Switch back to the `alloc_c_string()`-method once the API gets updated
            {
                const CharString dummy_binding = (param_bindings[i].operator String()).utf8();
                const char *binding = dummy_binding.get_data();
                sqlite3_bind_text(stmt, i + 1, binding, -1, SQLITE_TRANSIENT);
            }
            //sqlite3_bind_text(stmt, i + 1, (param_bindings[i].operator String()).alloc_c_string(), -1, SQLITE_TRANSIENT);
            break;

        case Variant::PACKED_BYTE_ARRAY:
        {
            // TODO: Figure out how to read from a PackedByteArray in GDExtension
            /*
            PackedByteArray binding = ((const PackedByteArray &)param_bindings[i]);
            PackedByteArray::Read r = binding.read();
            sqlite3_bind_blob64(stmt, i + 1, r.ptr(), binding.size(), SQLITE_TRANSIENT);
            */
            break;
        }

        default:
            UtilityFunctions::printerr("GDSQLite Error: Binding a parameter of type " + String(std::to_string(param_bindings[i].get_type()).c_str()) + " (TYPE_*) is not supported!");
            sqlite3_finalize(stmt);
            return false;
        }
    }

    if (verbose_mode)
    {
        char *expanded_sql = sqlite3_expanded_sql(stmt);
        UtilityFunctions::print(expanded_sql);
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
                // TODO: Figure out how to write to a PackedByteArray in GDExtension
                /*
                int bytes = sqlite3_column_bytes(stmt, i);
                PackedByteArray arr = PackedByteArray();
                arr.resize(bytes);
                PackedByteArray::Write write = arr.write();
                memcpy(write.ptr(), (char *)sqlite3_column_blob(stmt, i), bytes);
                column_value = arr;
                */
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

    /* Clean up and delete the resources used by the prepared statement */
    sqlite3_finalize(stmt);

    rc = sqlite3_errcode(db);
    zErrMsg = sqlite3_errmsg(db);
    error_message = String(zErrMsg);
    if (rc != SQLITE_OK)
    {
        UtilityFunctions::printerr(" --> SQL error: " + error_message);
        return false;
    }
    else if (verbose_mode)
    {
        UtilityFunctions::print(" --> Query succeeded");
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
            UtilityFunctions::printerr("GDSQLite Error: The field \"data_type\" is a required part of the table dictionary");
            return false;
        }
        query_string = query_string + (const String &)columns[i] + " ";
        //query_string += (const String &)columns[i] + " ";
        type_string = (const String &)column_dict["data_type"];
        if (type_string.to_lower().begins_with(integer_datatype))
        {
            query_string = query_string + String("INTEGER");
            //query_string += String("INTEGER");
        }
        else
        {
            query_string = query_string + type_string;
            //query_string += type_string;
        }

        /* Primary key check */
        if (column_dict.get("primary_key", false))
        {
            query_string = query_string + String(" PRIMARY KEY");
            //query_string += String(" PRIMARY KEY");
        }
        /* Default check */
        if (column_dict.has("default"))
        {
            query_string = query_string + String(" DEFAULT ") + (const String &)column_dict["default"];
            //query_string += String(" DEFAULT ") + (const String &)column_dict["default"];
        }
        /* Autoincrement check */
        if (column_dict.get("auto_increment", false))
        {
            query_string = query_string + String(" AUTOINCREMENT");
            //query_string += String(" AUTOINCREMENT");
        }
        /* Not null check */
        if (column_dict.get("not_null", false))
        {
            query_string = query_string + String(" NOT NULL");
            //query_string += String(" NOT NULL");
        }
        /* Apply foreign key constraint. */
        if (foreign_keys)
        {
            if (column_dict.get("foreign_key", false))
            {
                const String foreign_key_definition = (const String &)(column_dict["foreign_key"]);
                const Array foreign_key_elements = foreign_key_definition.split(".");
                if (foreign_key_elements.size() == 2)
                {
                    const String column_name = (const String &)(columns[i]);
                    const String foreign_key_table_name = (const String &)(foreign_key_elements[0]);
                    const String foreign_key_column_name = (const String &)(foreign_key_elements[1]);
                    key_string = key_string + String(", FOREIGN KEY (" + column_name + ") REFERENCES " + foreign_key_table_name + "(" + foreign_key_column_name + ")");
                    //key_string += String(", FOREIGN KEY (" + column_name + ") REFERENCES " + foreign_key_table_name + "(" + foreign_key_column_name + ")");
                }
            }
        }

        if (i != number_of_columns - 1)
        {
            query_string = query_string + ",";
            //query_string += ",";
        }
    }

    query_string = query_string + key_string + ");";
    //query_string += key_string + ");";

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
        key_string = key_string + (const String &)keys[i];
        //key_string += (const String &)keys[i];
        value_string = value_string + "?";
        //value_string += "?";
        if (i != number_of_keys - 1)
        {
            key_string = key_string + ",";
            //key_string += ",";
            value_string = value_string + ",";
            //value_string += ",";
        }
    }
    query_string = query_string + " (" + key_string + ") VALUES (" + value_string + ");";
    //query_string += " (" + key_string + ") VALUES (" + value_string + ");";

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
            UtilityFunctions::printerr("GDSQLite Error: All elements of the Array should be of type Dictionary");
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
            UtilityFunctions::printerr("GDSQLite Error: All elements of the Array should be of type String");
            return query_result;
        }
        query_string = query_string + (const String &)p_columns_array[i];
        //query_string += (const String &)p_columns_array[i];

        if (i != number_of_columns - 1)
        {
            query_string = query_string + ", ";
            //query_string += ", ";
        }
    }
    query_string = query_string + " FROM " + p_name;
    //query_string += " FROM " + p_name;
    if (!p_conditions.is_empty())
    {
        query_string = query_string + " WHERE " + p_conditions;
        //query_string += " WHERE " + p_conditions;
    }
    query_string = query_string + ";";
    //query_string += ";";

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
    query_string = query_string + "UPDATE " + p_name + " SET ";
    //query_string += "UPDATE " + p_name + " SET ";

    for (int i = 0; i <= number_of_keys - 1; i++)
    {
        query_string = query_string + (const String &)keys[i] + "=?";
        //query_string += (const String &)keys[i] + "=?";
        param_bindings.append(values[i]);
        if (i != number_of_keys - 1)
        {
            query_string = query_string + ", ";
            //query_string += ", ";
        }
    }
    query_string = query_string + " WHERE " + p_conditions + ";";
    //query_string += " WHERE " + p_conditions + ";";

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
    if (!p_conditions.is_empty() && (p_conditions != (const String &)"*"))
    {
        query_string = query_string + " WHERE " + p_conditions;
        //query_string += " WHERE " + p_conditions;
    }
    query_string = query_string + ";";
    //query_string += ";";

    success = query(query_string);
    /* Stop the error_message from being overwritten! */
    String previous_error_message = error_message;
    query("END TRANSACTION;");
    error_message = previous_error_message;
    return success;
}

// Properties.
void SQLite::set_verbose_mode(const bool &p_verbose_mode)
{
    verbose_mode = p_verbose_mode;
}

bool SQLite::get_verbose_mode() const
{
    return verbose_mode;
}

void SQLite::set_foreign_keys(const bool &p_foreign_keys)
{
    foreign_keys = p_foreign_keys;
}

bool SQLite::get_foreign_keys() const
{
    return foreign_keys;
}

void SQLite::set_read_only(const bool &p_read_only)
{
    read_only = p_read_only;
}

bool SQLite::get_read_only() const
{
    return read_only;
}

void SQLite::set_path(const String &p_path)
{
    path = p_path;
}

String SQLite::get_path() const
{
    return path;
}

void SQLite::set_error_message(const String &p_error_message)
{
    error_message = p_error_message;
}

String SQLite::get_error_message() const
{
    return error_message;
}

void SQLite::set_default_extension(const String &p_default_extension)
{
    default_extension = p_default_extension;
}

String SQLite::get_default_extension() const
{
    return default_extension;
}

void SQLite::set_query_result(const Array &p_query_result)
{
    query_result = p_query_result;
}

Array SQLite::get_query_result() const
{
    return query_result;
}