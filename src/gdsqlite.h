#ifndef GDSQLITE_H
#define GDSQLITE_H

#include <Godot.hpp>
#include <Reference.hpp>
#include <FuncRef.hpp>
#include <ProjectSettings.hpp>
#include <Marshalls.hpp>
#include <Directory.hpp>
#include <JSON.hpp>
#include <JSONParseResult.hpp>

#include <fstream>
#include <vector>
#include <sstream>
#include <sqlite/sqlite3.h>
#include <helpers/current_function.h>
#include <vfs/gdsqlite_vfs.h>

namespace godot
{

#define GODOT_LOG(level, message)                                                  \
    switch (level)                                                                 \
    {                                                                              \
    case 0:                                                                        \
        Godot::print(message);                                                     \
        break;                                                                     \
    case 1:                                                                        \
        Godot::print_warning(message, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__); \
        break;                                                                     \
    case 2:                                                                        \
        Godot::print_error(message, BOOST_CURRENT_FUNCTION, __FILE__, __LINE__);   \
        break;                                                                     \
    }

    enum OBJECT_TYPE
    {
        TABLE,
        TRIGGER
    };
    struct object_struct
    {
        String name, sql;
        OBJECT_TYPE type;
        Array base64_columns, row_array;
    };

    class SQLite : public Reference
    {
        GODOT_CLASS(SQLite, Reference)

    private:
        sqlite3 *db;
        std::vector<Ref<FuncRef>> function_registry;

        Dictionary deep_copy(Dictionary p_dict);
        Variant get_with_default(Dictionary p_dict, String p_key, Variant p_default);
        bool validate_json(Array import_json, std::vector<object_struct> &tables_to_import);

    public:
        enum VerbosityLevel
        {
            QUIET = 0,
            NORMAL = 1,
            VERBOSE = 2,
            VERY_VERBOSE = 3
        };

        int last_insert_rowid, verbosity_level;
        bool foreign_keys, read_only;
        String path, error_message, default_extension;
        Array query_result;

        static void _register_methods();

        SQLite();
        ~SQLite();

        void _init();

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

        bool create_function(String p_name, Ref<FuncRef> p_func_ref, int p_argc);

        bool import_from_json(String import_path);
        bool export_to_json(String export_path);

        int get_autocommit();

        void set_last_insert_rowid(int p_last_row_id);
        int get_last_insert_rowid();

        void set_verbosity_level(int p_verbosity_level);
        int get_verbosity_level();

        void set_verbose_mode(bool p_verbose_mode);
        bool get_verbose_mode();

        void set_query_result(Array p_query_result);
        Array get_query_result();

        Array get_query_result_by_reference();
    };

}

#endif
