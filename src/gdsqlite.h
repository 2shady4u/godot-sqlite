#ifndef GDSQLITE_H
#define GDSQLITE_H

#include <Godot.hpp>
#include <Reference.hpp>
#include <ProjectSettings.hpp>
#include <JSON.hpp>
#include <JSONParseResult.hpp>

#include <fstream>
#include <vector>
#include <sstream>
#include <sqlite/sqlite3.h>

namespace godot {

struct table_struct
{
	String name, sql;
    Array row_array;
};

class SQLite : public Reference {
    GODOT_CLASS(SQLite, Reference)

private:
    String path;
    bool verbose_mode;

    Dictionary deep_copy(Dictionary p_dict);
    bool validate_json(Array import_json, std::vector<table_struct> &tables_to_import);

public:
    sqlite3 *db;
    Array query_result;

    static void _register_methods();

    SQLite();
    ~SQLite();

    void _init();

    bool open_db();
    bool import_from_json(String import_path);
    bool export_to_json(String export_path);
    void close_db();
    bool query(String p_query);

    bool create_table(String p_name, Dictionary p_table_dict);
    bool drop_table(String p_name);

    bool insert_row(String p_name, Dictionary p_row_dict);
    bool insert_rows(String p_name, Array p_row_array);

    Array select_rows(String p_name, String p_conditions, Array p_columns_array);
    bool update_rows(String p_name, String p_conditions, Dictionary p_updated_row_dict);
    bool delete_rows(String p_name, String p_conditions);
    void set_path(String p_path);
    String get_path();
};

}

#endif