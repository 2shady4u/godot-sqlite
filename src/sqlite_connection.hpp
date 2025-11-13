#ifndef SQLITE_CONNECTION_CLASS_H
#define SQLITE_CONNECTION_CLASS_H

#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/marshalls.hpp>
#include <godot_cpp/classes/project_settings.hpp>

#include <sqlite/sqlite3.h>
#include <vfs/gdsqlite_vfs.hpp>
#include <sqlite_connection_params.hpp>
#include <sqlite_enums.hpp>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>

namespace godot {
enum ObjectType {
	TABLE,
	INDEX,
	VIEW,
	TRIGGER
};
struct ObjectStruct {
	String name, sql;
	ObjectType type;
	Array base64_columns, row_array;
};

class SQLiteConnection : public RefCounted {
	GDCLASS(SQLiteConnection, RefCounted)

	static SQLiteEnums::ResultCode open_error;

private:
	static bool validate_json(const Array &import_json, std::vector<ObjectStruct> &tables_to_import);
	static bool validate_table_dict(const Dictionary &p_table_dict);
	int backup_database(sqlite3 *source_db, sqlite3 *destination_db);
	void remove_shadow_tables(Array &p_array);

	String normalize_path(const String p_path, const bool read_only) const;

	sqlite3 *db;
	std::vector<std::unique_ptr<Callable>> function_registry;

	SQLiteEnums::VerbosityLevel verbosity_level = SQLiteEnums::VerbosityLevel::NORMAL;
	bool foreign_keys = false;
	bool read_only = false;
	String path = "database.db";
	String error_message = "";

protected:
	SQLiteConnection();
	static void _bind_methods();

public:
	static Ref<SQLiteConnection> open(const Ref<SQLiteConnectionParams> &params);
	~SQLiteConnection();

	// Functions.
	SQLiteEnums::ResultCode query(const String &p_query, Array p_query_result = Array());
	SQLiteEnums::ResultCode query_with_bindings(const String &p_query, Array param_bindings, Array p_query_result = Array());

	SQLiteEnums::ResultCode create_table(const StringName &p_table_name, const Dictionary &p_table_dict);
	SQLiteEnums::ResultCode drop_table(const StringName &p_table_name);

	SQLiteEnums::ResultCode backup_to(String destination_path);
	SQLiteEnums::ResultCode restore_from(String source_path);

	SQLiteEnums::ResultCode insert_row(const StringName &p_table_name, const Dictionary &p_row_dict, Array p_query_result = Array());
	SQLiteEnums::ResultCode insert_rows(const StringName &p_table_name, const Array &p_row_array, Array p_query_result = Array());

	SQLiteEnums::ResultCode select_rows(const StringName &p_table_name, const String &p_conditions, const Array &p_columns_array, Array p_query_result = Array());
	SQLiteEnums::ResultCode update_rows(const StringName &p_table_name, const String &p_conditions, const Dictionary &p_updated_row_dict, Array p_query_result = Array());
	SQLiteEnums::ResultCode delete_rows(const StringName &p_table_name, const String &p_conditions, Array p_query_result = Array());

	SQLiteEnums::ResultCode create_function(const String &p_name, const Callable &p_callable, int p_argc);

	SQLiteEnums::ResultCode import_from_json(String import_path);
	SQLiteEnums::ResultCode export_to_json(String export_path);

	SQLiteEnums::ResultCode import_from_buffer(PackedByteArray json_buffer);
	PackedByteArray export_to_buffer();

	int get_autocommit() const;
	int compileoption_used(const String &option_name) const;

	int load_extension(const String &p_path, const String &p_init_func_name);
	int enable_load_extension(const bool &p_onoff);

	// Properties.
	void set_last_insert_rowid(const int64_t &p_last_insert_rowid);
	int64_t get_last_insert_rowid() const;

	void set_verbosity_level(SQLiteEnums::VerbosityLevel p_verbosity_level);
	SQLiteEnums::VerbosityLevel get_verbosity_level() const;

	void set_foreign_keys(const bool &p_foreign_keys);
	bool get_foreign_keys() const;

	bool get_read_only() const;

	String get_path() const;

	void set_error_message(const String &p_error_message);
	String get_error_message() const;

	static SQLiteEnums::ResultCode get_open_error() {
        return open_error;
    }
};

} //namespace godot

#endif // ! SQLITE_CONNECTION_CLASS_H
