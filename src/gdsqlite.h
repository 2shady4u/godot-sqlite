#ifndef GDSQLITE_H
#define GDSQLITE_H

#include <Godot.hpp>
#include <Reference.hpp>
#include <ProjectSettings.hpp>
#include "../sqlite/sqlite3.h"

namespace godot {

class SQLite : public Reference {
    GODOT_CLASS(SQLite, Reference)

private:
    String path;
    bool verbose_mode;

public:
    sqlite3 *db;
    Array query_result;

    static void _register_methods();

    SQLite();
    ~SQLite();

    void _init();

    bool open_db();
    void close_db();
    bool query(String p_query);

    bool create_table(String p_table, Dictionary p_table_dict);
    bool drop_table(String p_table);

    bool insert_row(String p_table, Dictionary p_row_dict);
    bool insert_rows(String p_table, Array p_row_array);

    Array select_rows(String p_table, String p_conditions, Array p_columns_array);
    bool update_rows(String p_table, String p_conditions, Dictionary p_updated_row_dict);
    bool delete_rows(String p_table, String p_conditions);
    void set_path(String p_path);
    String get_path();
};

}

#endif