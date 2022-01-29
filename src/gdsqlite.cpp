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

    // Properties.
    ClassDB::bind_method(D_METHOD("set_path", "path"), &SQLite::set_path);
    ClassDB::bind_method(D_METHOD("get_path"), &SQLite::get_path);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "path"), "set_path", "get_path");
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

// Properties.
void SQLite::set_path(const String &p_path) {
	path = p_path;
}

String SQLite::get_path() const {
	return path;
}