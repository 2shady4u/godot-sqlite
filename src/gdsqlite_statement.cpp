#include "gdsqlite_statement.hpp"
#include <cstring>

using namespace godot;

void SQLiteStatement::_bind_methods() {
	ClassDB::bind_method(D_METHOD("bind", "index", "value"), &SQLiteStatement::bind);
	ClassDB::bind_method(D_METHOD("bind_all", "values"), &SQLiteStatement::bind_all);
	ClassDB::bind_method(D_METHOD("bind_named", "values"), &SQLiteStatement::bind_named);
	ClassDB::bind_method(D_METHOD("clear_bindings"), &SQLiteStatement::clear_bindings);

	ClassDB::bind_method(D_METHOD("reset"), &SQLiteStatement::reset);
	ClassDB::bind_method(D_METHOD("execute"), &SQLiteStatement::execute);
	ClassDB::bind_method(D_METHOD("step"), &SQLiteStatement::step);
	ClassDB::bind_method(D_METHOD("fetch_all"), &SQLiteStatement::fetch_all);
	ClassDB::bind_method(D_METHOD("get_row"), &SQLiteStatement::get_row);
	ClassDB::bind_method(D_METHOD("get_column_names"), &SQLiteStatement::get_column_names);
	ClassDB::bind_method(D_METHOD("get_parameter_count"), &SQLiteStatement::get_parameter_count);

	ClassDB::bind_method(D_METHOD("finalize"), &SQLiteStatement::finalize);
	ClassDB::bind_method(D_METHOD("is_valid"), &SQLiteStatement::is_valid);
	ClassDB::bind_method(D_METHOD("get_status"), &SQLiteStatement::get_status);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "status"), "", "get_status");

	ClassDB::bind_method(D_METHOD("get_error_message"), &SQLiteStatement::get_error_message);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "error_message"), "", "get_error_message");

	BIND_ENUM_CONSTANT(UNINITIALIZED);
	BIND_ENUM_CONSTANT(INITIALIZED);
	BIND_ENUM_CONSTANT(FINALIZED);
	BIND_ENUM_CONSTANT(CONNECTION_FINALIZED);
}

SQLiteStatement::~SQLiteStatement() {
	finalize();
}

void SQLiteStatement::initialize(sqlite3 *p_db, sqlite3_stmt *p_stmt) {
	db = p_db;
	stmt = p_stmt;
	status = (stmt != nullptr && db != nullptr) ? INITIALIZED : UNINITIALIZED;
	reset_row_state();
	column_names.clear();
}

void SQLiteStatement::connection_finalized() {
	release_statement(CONNECTION_FINALIZED);
	db = nullptr;
}

bool SQLiteStatement::check_valid(const char *p_method_name) {
	if (status == FINALIZED) {
		error_message = "Statement was explicitly finalized and can no longer be used.";
		ERR_PRINT("GDSQLite Error: Cannot call " + String(p_method_name) + " because the statement was explicitly finalized.");
		return false;
	}

	if (status == CONNECTION_FINALIZED || db == nullptr) {
		error_message = "Statement is invalid because the associated database connection is no longer available.";
		ERR_PRINT("GDSQLite Error: Cannot call " + String(p_method_name) + " because the associated database is no longer available.");
		return false;
	}

	if (status == UNINITIALIZED) {
		error_message = "Statement is uninitialized.";
		ERR_PRINT("GDSQLite Error: Cannot call " + String(p_method_name) + " on an invalid statement.");
		return false;
	}

	if (status != INITIALIZED || stmt == nullptr) {
		error_message = "Statement is in an invalid internal state.";
		ERR_PRINT("GDSQLite Error: Cannot call " + String(p_method_name) + " on a corrupted statement state.");
		return false;
	}

	return true;
}

void SQLiteStatement::release_statement(StatementStatus p_next_status) {
	switch (p_next_status) {
		case FINALIZED:
		case CONNECTION_FINALIZED:
			status = p_next_status;
			break;

		default:
			ERR_PRINT("GDSQLite Error: Unexpected release_statement status transition.");
			break;
	}

	if (stmt == nullptr) {
		reset_row_state();
		column_names.clear();
		return;
	}

	int rc = sqlite3_finalize(stmt);
	stmt = nullptr;

	if (status == INITIALIZED) {
		status = UNINITIALIZED;
	}

	reset_row_state();
	column_names.clear();

	if (db != nullptr && rc != SQLITE_OK) {
		error_message = String::utf8(sqlite3_errmsg(db));
		ERR_PRINT("GDSQLite Error: " + error_message);
	}
}

void SQLiteStatement::cache_column_names() {
	if (stmt == nullptr || !column_names.is_empty()) {
		return;
	}

	int column_count = sqlite3_column_count(stmt);

	for (int i = 0; i < column_count; i++) {
		const char *column_name = sqlite3_column_name(stmt, i);
		column_names.append(String::utf8(column_name));
	}
}

void SQLiteStatement::reset_row_state() {
	has_row = false;
	current_row.clear();
}

Dictionary SQLiteStatement::marshal_current_row() const {
	Dictionary row;

	if (stmt == nullptr) {
		return row;
	}

	int column_count = sqlite3_column_count(stmt);

	for (int i = 0; i < column_count; i++) {
		StringName column_name;

		if (i < column_names.size()) {
			column_name = StringName(column_names[i]);
		} else {
			column_name = StringName(String::utf8(sqlite3_column_name(stmt, i)));
		}

		Variant column_value;

		switch (sqlite3_column_type(stmt, i)) {
			case SQLITE_INTEGER:
				column_value = (int64_t)sqlite3_column_int64(stmt, i);
				break;

			case SQLITE_FLOAT:
				column_value = sqlite3_column_double(stmt, i);
				break;

			case SQLITE_TEXT:
				column_value = String::utf8((const char *)sqlite3_column_text(stmt, i));
				break;

			case SQLITE_BLOB: {
				int bytes = sqlite3_column_bytes(stmt, i);
				PackedByteArray arr;
				arr.resize(bytes);
				if (bytes > 0) {
					memcpy(arr.ptrw(), sqlite3_column_blob(stmt, i), bytes);
				}
				column_value = arr;
				break;
			}

			case SQLITE_NULL:
				column_value = Variant();
				break;

			default:
				column_value = Variant();
				break;
		}

		row[column_name] = column_value;
	}
	return row;
}

bool SQLiteStatement::bind_sqlite_parameter(const Variant &p_binding_value, int p_sqlite_index) {
	switch (p_binding_value.get_type()) {
		case Variant::NIL:
			sqlite3_bind_null(stmt, p_sqlite_index);
			break;

		case Variant::BOOL:
		case Variant::INT:
			sqlite3_bind_int64(stmt, p_sqlite_index, int64_t(p_binding_value));
			break;

		case Variant::FLOAT:
			sqlite3_bind_double(stmt, p_sqlite_index, p_binding_value);
			break;

		case Variant::STRING:
		case Variant::STRING_NAME: {
			const CharString utf8_binding = (p_binding_value.operator String()).utf8();
			sqlite3_bind_text(stmt, p_sqlite_index, utf8_binding.get_data(), -1, SQLITE_TRANSIENT);
			break;
		}

		case Variant::PACKED_BYTE_ARRAY: {
			PackedByteArray binding = ((const PackedByteArray &)p_binding_value);
			if (binding.size() == 0) {
				sqlite3_bind_null(stmt, p_sqlite_index);
			} else {
				sqlite3_bind_blob64(stmt, p_sqlite_index, binding.ptr(), binding.size(), SQLITE_TRANSIENT);
			}
			break;
		}

		default:
			error_message = "Binding a parameter of this Variant type is not supported.";
			ERR_PRINT("GDSQLite Error: " + error_message);
			return false;
	}

	return true;
}

bool SQLiteStatement::bind(const int64_t &p_index, const Variant &p_binding_value) {
	if (!check_valid("bind")) {
		return false;
	}

	int parameter_count = sqlite3_bind_parameter_count(stmt);

	if (p_index < 0 || p_index >= parameter_count) {
		error_message = "Binding index is out of range.";
		ERR_PRINT("GDSQLite Error: " + error_message);
		return false;
	}

	return bind_sqlite_parameter(p_binding_value, (int)p_index + 1);
}

bool SQLiteStatement::bind_all(const Array &p_values) {
	if (!check_valid("bind_all")) {
		return false;
	}

	int parameter_count = sqlite3_bind_parameter_count(stmt);

	if (p_values.size() < parameter_count) {
		error_message = "Insufficient number of parameters to satisfy required bindings in statement.";
		ERR_PRINT("GDSQLite Error: " + error_message);
		return false;
	}

	for (int i = 0; i < parameter_count; i++) {
		if (!bind_sqlite_parameter(p_values[i], i + 1)) {
			return false;
		}
	}

	if (p_values.size() > parameter_count) {
		WARN_PRINT("GDSQLite Warning: Provided number of bindings exceeded the required number in statement.");
	}

	return true;
}

bool SQLiteStatement::bind_named(const Dictionary &p_values) {
	if (!check_valid("bind_named")) {
		return false;
	}

	int parameter_count = sqlite3_bind_parameter_count(stmt);

	for (int i = 0; i < parameter_count; i++) {
		const char *parameter_name = sqlite3_bind_parameter_name(stmt, i + 1);

		if (parameter_name == nullptr) {
			error_message = "Named binding failed because a parameter in the statement is nameless.";
			ERR_PRINT("GDSQLite Error: " + error_message);
			return false;
		}

		String parameter_key = String::utf8(parameter_name + 1);
		Variant value;

		if (p_values.has(parameter_key)) {
			value = p_values[parameter_key];
		} else if (p_values.has(StringName(parameter_key))) {
			value = p_values[StringName(parameter_key)];
		} else {
			error_message = "Missing named parameter: " + parameter_key;
			ERR_PRINT("GDSQLite Error: " + error_message);
			return false;
		}

		if (!bind_sqlite_parameter(value, i + 1)) {
			return false;
		}
	}

	return true;
}

void SQLiteStatement::clear_bindings() {
	if (!check_valid("clear_bindings")) {
		return;
	}

	sqlite3_clear_bindings(stmt);
}

bool SQLiteStatement::reset() {
	if (!check_valid("reset")) {
		return false;
	}

	reset_row_state();
	int rc = sqlite3_reset(stmt);

	if (rc != SQLITE_OK) {
		error_message = String::utf8(sqlite3_errmsg(db));
		ERR_PRINT("GDSQLite Error: " + error_message);
		return false;
	}

	return true;
}

bool SQLiteStatement::execute() {
	if (!check_valid("execute")) {
		return false;
	}

	while (true) {
		int64_t rc = step();

		if (rc == SQLITE_ROW) {
			continue;
		}

		return rc == SQLITE_DONE;
	}
}

int64_t SQLiteStatement::step() {
	if (!check_valid("step")) {
		return SQLITE_MISUSE;
	}

	cache_column_names();
	int rc = sqlite3_step(stmt);

	if (rc == SQLITE_ROW) {
		has_row = true;
		current_row = marshal_current_row();
		return rc;
	}

	if (rc == SQLITE_DONE) {
		reset_row_state();
		return rc;
	}

	reset_row_state();
	error_message = String::utf8(sqlite3_errmsg(db));
	ERR_PRINT("GDSQLite Error: " + error_message);
	return rc;
}

TypedArray<Dictionary> SQLiteStatement::fetch_all() {
	TypedArray<Dictionary> rows;

	if (!check_valid("fetch_all")) {
		return rows;
	}

	while (true) {
		int64_t rc = step();

		if (rc == SQLITE_ROW) {
			rows.append(current_row.duplicate(true));
			continue;
		}

		break;
	}

	return rows;
}

Dictionary SQLiteStatement::get_row() const {
	if (!has_row) {
		return Dictionary();
	}

	return current_row.duplicate(true);
}

PackedStringArray SQLiteStatement::get_column_names() {
	if (status == INITIALIZED && column_names.is_empty()) {
		cache_column_names();
	}

	return column_names;
}

int64_t SQLiteStatement::get_parameter_count() const {
	if (status != INITIALIZED) {
		return 0;
	}

	return sqlite3_bind_parameter_count(stmt);
}

void SQLiteStatement::finalize() {
	release_statement(FINALIZED);
}

bool SQLiteStatement::is_valid() const {
	return status == INITIALIZED;
}

int64_t SQLiteStatement::get_status() const {
	return status;
}

String SQLiteStatement::get_error_message() const {
	return error_message;
}
