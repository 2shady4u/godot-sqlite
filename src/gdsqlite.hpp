#ifndef SQLITE_CLASS_H
#define SQLITE_CLASS_H

#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/core/binder_common.hpp>
#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/json.hpp>
#include <godot_cpp/classes/marshalls.hpp>
#include <godot_cpp/classes/project_settings.hpp>

#include "gdsqlite_statement.hpp"
#include <sqlite/sqlite3.h>
#include <cstring>
#include <fstream>
#include <memory>
#include <sstream>
#include <vector>
#include <vfs/gdsqlite_vfs.hpp>

namespace godot {
enum OBJECT_TYPE {
	TABLE,
	INDEX,
	VIEW,
	TRIGGER
};
struct object_struct {
	String name, sql;
	OBJECT_TYPE type;
	Array base64_columns, row_array;
};

class SQLite : public RefCounted {
	GDCLASS(SQLite, RefCounted)

private:
	bool validate_json(const Array &import_json, std::vector<object_struct> &tables_to_import);
	bool validate_table_dict(const Dictionary &p_table_dict);
	int backup_database(sqlite3 *source_db, sqlite3 *destination_db);
	void remove_shadow_tables(Array &p_array);
	bool prepare_statement(const CharString &p_query, sqlite3_stmt **out_stmt, const char **pzTail);
	bool bind_parameter(Variant binding_value, sqlite3_stmt *stmt, int i);
	bool execute_statement(sqlite3_stmt *stmt);
	void track_statement_instance(uint64_t p_instance_id);
	void prune_statement_instances();
	void finalize_statements();

	String normalize_path(const String p_path, const bool read_only) const;

	sqlite3 *db;
	std::vector<std::unique_ptr<Callable>> function_registry;
	std::vector<uint64_t> statement_instance_ids;

	int64_t verbosity_level = 1;
	bool foreign_keys = false;
	bool read_only = false;
	String path = "default";
	String error_message = "";
	String default_extension = "db";
	TypedArray<Dictionary> query_result = TypedArray<Dictionary>();

protected:
	static void _bind_methods();

public:
	// Constants.
	enum VerbosityLevel {
		QUIET = 0,
		NORMAL = 1,
		VERBOSE = 2,
		VERY_VERBOSE = 3
	};

	SQLite();
	~SQLite();

	// Functions.
	bool open_db();
	bool close_db();
	bool query(const String &p_query);
	Ref<SQLiteStatement> prepare(const String &p_query);
	bool query_with_bindings(const String &p_query, Array param_bindings);
	bool query_with_named_bindings(const String &p_query, Dictionary param_bindings);

	bool create_table(const String &p_name, const Dictionary &p_table_dict);
	bool drop_table(const String &p_name);

	bool backup_to(String destination_path);
	bool restore_from(String source_path);

	bool insert_row(const String &p_name, const Dictionary &p_row_dict);
	bool insert_rows(const String &p_name, const Array &p_row_array);

	Array select_rows(const String &p_name, const String &p_conditions, const Array &p_columns_array);
	bool update_rows(const String &p_name, const String &p_conditions, const Dictionary &p_updated_row_dict);
	bool delete_rows(const String &p_name, const String &p_conditions);

	bool create_function(const String &p_name, const Callable &p_callable, int p_argc);

	bool import_from_json(String import_path);
	bool export_to_json(String export_path);

	bool import_from_buffer(PackedByteArray json_buffer);
	PackedByteArray export_to_buffer();

	int get_autocommit() const;
	int compileoption_used(const String &option_name) const;

	int load_extension(const String &p_path, const String &p_init_func_name);
	int enable_load_extension(const bool &p_onoff);

	// Properties.
	void set_last_insert_rowid(const int64_t &p_last_insert_rowid);
	int64_t get_last_insert_rowid() const;

	void set_verbosity_level(const int64_t &p_verbosity_level);
	int64_t get_verbosity_level() const;

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

	void set_query_result(const TypedArray<Dictionary> &p_query_result);
	TypedArray<Dictionary> get_query_result() const;

	TypedArray<Dictionary> get_query_result_by_reference() const;
};

} //namespace godot

VARIANT_ENUM_CAST(SQLite::VerbosityLevel);

#endif // ! SQLITE_CLASS_H
