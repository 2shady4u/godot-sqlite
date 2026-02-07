#ifndef GDSQLITE_STATEMENT_H
#define GDSQLITE_STATEMENT_H

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

#include <sqlite/sqlite3.h>

namespace godot {

class SQLiteStatement : public RefCounted {
	GDCLASS(SQLiteStatement, RefCounted)

public:
	enum StatementStatus : int {
		UNINITIALIZED = 0,
		INITIALIZED = 1,
		FINALIZED = 2,
		CONNECTION_FINALIZED = 3
	};

private:
	sqlite3 *db = nullptr;
	sqlite3_stmt *stmt = nullptr;
	StatementStatus status = UNINITIALIZED;
	bool has_row = false;
	String error_message = "";
	Dictionary current_row;
	PackedStringArray column_names;

	bool check_valid(const char *p_method_name);
	void release_statement(StatementStatus p_next_status);
	void cache_column_names();
	void reset_row_state();
	Dictionary marshal_current_row() const;
	bool bind_sqlite_parameter(const Variant &p_binding_value, int p_sqlite_index);

protected:
	static void _bind_methods();

public:
	SQLiteStatement() = default;
	~SQLiteStatement();

	void initialize(sqlite3 *p_db, sqlite3_stmt *p_stmt);
	void connection_finalized();

	bool bind(const int64_t &p_index, const Variant &p_binding_value);
	bool bind_all(const Array &p_values);
	bool bind_named(const Dictionary &p_values);
	void clear_bindings();

	bool reset();
	bool execute();
	int64_t step();
	TypedArray<Dictionary> fetch_all();
	Dictionary get_row() const;
	PackedStringArray get_column_names();
	int64_t get_parameter_count() const;

	void finalize();
	bool is_valid() const;
	int64_t get_status() const;

	String get_error_message() const;
};

} //namespace godot

VARIANT_ENUM_CAST(SQLiteStatement::StatementStatus);

#endif // ! GDSQLITE_STATEMENT_H
