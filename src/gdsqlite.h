#ifndef SQLITE_CLASS_H
#define SQLITE_CLASS_H

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/viewport.hpp>

#include <godot_cpp/core/binder_common.hpp>

#include <sqlite/sqlite3.h>

namespace godot
{
    class SQLite : public RefCounted
    {
        GDCLASS(SQLite, RefCounted)

    private:
        sqlite3 *db;
        int last_insert_rowid;
        bool verbose_mode, foreign_keys, read_only;
        String path, error_message, default_extension;
        Array query_result;

    protected:
        static void _bind_methods();

    public:
        SQLite();
        ~SQLite();

        // Functions.
        bool open_db();
        void close_db();

        // Property.
	    void set_path(const String &p_path);
	    String get_path() const;
    };

}

#endif // ! SQLITE_CLASS_H
