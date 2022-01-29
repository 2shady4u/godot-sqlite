#ifndef SQLITE_CLASS_H
#define SQLITE_CLASS_H

#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/marshalls.hpp>
#include <godot_cpp/classes/json.hpp>

#include <fstream>
#include <vector>
#include <sstream>
#include <sqlite/sqlite3.h>

namespace godot
{
    class SQLite : public RefCounted
    {
        GDCLASS(SQLite, RefCounted)

    private:
        sqlite3 *db;

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
        bool query(String p_query);
        bool query_with_bindings(String p_query, Array param_bindings);

        bool create_table(String p_name, Dictionary p_table_dict);
        bool drop_table(String p_name);

        bool insert_row(String p_name, Dictionary p_row_dict);
        bool insert_rows(String p_name, Array p_row_array);

        Array select_rows(String p_name, String p_conditions, Array p_columns_array);
        bool update_rows(String p_name, String p_conditions, Dictionary p_updated_row_dict);
        bool delete_rows(String p_name, String p_conditions);

        // Properties.
        void set_last_insert_rowid(const int &p_last_insert_rowid);
        int get_last_insert_rowid() const;

        void set_verbose_mode(const bool &p_verbose_mode);
        bool get_verbose_mode() const;

        void set_foreign_keys(const bool &p_foreign_keys);
        bool get_foreign_keys() const;

        void set_read_only(const bool &p_read_only);
        bool get_read_only() const;

        void set_path(const String &p_path);
        String get_path() const;

        void set_error_message(const String &p_error_message);
        String get_error_message() const;

        void set_default_extension(const String &p_default_extension);
        String get_default_extension() const;

        void set_query_result(const Array &p_query_result);
        Array get_query_result() const;
    };

}

#endif // ! SQLITE_CLASS_H
