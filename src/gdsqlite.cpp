#include "gdsqlite.h"

using namespace godot;

void SQLite::_bind_methods() {
	// Methods.
	ClassDB::bind_method(D_METHOD("open_db"), &SQLite::open_db);
	ClassDB::bind_method(D_METHOD("close_db"), &SQLite::close_db);
	ClassDB::bind_method(D_METHOD("query", "query_string"), &SQLite::query);
	ClassDB::bind_method(D_METHOD("query_with_bindings", "query_string", "param_bindings"), &SQLite::query_with_bindings);

	ClassDB::bind_method(D_METHOD("create_table", "table_name", "table_data"), &SQLite::create_table);
	ClassDB::bind_method(D_METHOD("drop_table", "table_name"), &SQLite::drop_table);

	ClassDB::bind_method(D_METHOD("backup_to", "destination"), &SQLite::backup_to);
	ClassDB::bind_method(D_METHOD("restore_from", "source"), &SQLite::restore_from);

	ClassDB::bind_method(D_METHOD("insert_row", "table_name", "row_data"), &SQLite::insert_row);
	ClassDB::bind_method(D_METHOD("insert_rows", "table_name", "row_array"), &SQLite::insert_rows);

	ClassDB::bind_method(D_METHOD("select_rows", "table_name", "conditions", "columns"), &SQLite::select_rows);
	ClassDB::bind_method(D_METHOD("update_rows", "table_name", "conditions", "row_data"), &SQLite::update_rows);
	ClassDB::bind_method(D_METHOD("delete_rows", "table_name", "conditions"), &SQLite::delete_rows);

	ClassDB::bind_method(D_METHOD("create_function", "function_name", "callable", "arguments"), &SQLite::create_function);

	ClassDB::bind_method(D_METHOD("import_from_json", "import_path"), &SQLite::import_from_json);
	ClassDB::bind_method(D_METHOD("export_to_json", "export_path"), &SQLite::export_to_json);
	ClassDB::bind_method(D_METHOD("import_from_buffer", "json_buffer"), &SQLite::import_from_buffer);
	ClassDB::bind_method(D_METHOD("export_to_buffer"), &SQLite::export_to_buffer);

	ClassDB::bind_method(D_METHOD("get_autocommit"), &SQLite::get_autocommit);
	ClassDB::bind_method(D_METHOD("compileoption_used", "option_name"), &SQLite::compileoption_used);

	ClassDB::bind_method(D_METHOD("enable_load_extension", "onoff"), &SQLite::enable_load_extension);
	ClassDB::bind_method(D_METHOD("load_extension", "extension_path", "entrypoint"), &SQLite::load_extension, DEFVAL("sqlite3_extension_init"));

	// Properties.
	ClassDB::bind_method(D_METHOD("set_last_insert_rowid", "last_insert_rowid"), &SQLite::set_last_insert_rowid);
	ClassDB::bind_method(D_METHOD("get_last_insert_rowid"), &SQLite::get_last_insert_rowid);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "last_insert_rowid"), "set_last_insert_rowid", "get_last_insert_rowid");

	ClassDB::bind_method(D_METHOD("set_verbosity_level", "verbosity_level"), &SQLite::set_verbosity_level);
	ClassDB::bind_method(D_METHOD("get_verbosity_level"), &SQLite::get_verbosity_level);
	ADD_PROPERTY(PropertyInfo(Variant::INT, "verbosity_level"), "set_verbosity_level", "get_verbosity_level");

	ClassDB::bind_method(D_METHOD("set_foreign_keys", "foreign_keys"), &SQLite::set_foreign_keys);
	ClassDB::bind_method(D_METHOD("get_foreign_keys"), &SQLite::get_foreign_keys);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "foreign_keys"), "set_foreign_keys", "get_foreign_keys");

	ClassDB::bind_method(D_METHOD("set_read_only", "read_only"), &SQLite::set_read_only);
	ClassDB::bind_method(D_METHOD("get_read_only"), &SQLite::get_read_only);
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "read_only"), "set_read_only", "get_read_only");

	ClassDB::bind_method(D_METHOD("set_path", "path"), &SQLite::set_path);
	ClassDB::bind_method(D_METHOD("get_path"), &SQLite::get_path);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "path"), "set_path", "get_path");

	ClassDB::bind_method(D_METHOD("set_error_message", "error_message"), &SQLite::set_error_message);
	ClassDB::bind_method(D_METHOD("get_error_message"), &SQLite::get_error_message);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "error_message"), "set_error_message", "get_error_message");

	ClassDB::bind_method(D_METHOD("set_default_extension", "default_extension"), &SQLite::set_default_extension);
	ClassDB::bind_method(D_METHOD("get_default_extension"), &SQLite::get_default_extension);
	ADD_PROPERTY(PropertyInfo(Variant::STRING, "default_extension"), "set_default_extension", "get_default_extension");

	ClassDB::bind_method(D_METHOD("set_query_result", "query_result"), &SQLite::set_query_result);
	ClassDB::bind_method(D_METHOD("get_query_result"), &SQLite::get_query_result);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "query_result", PROPERTY_HINT_ARRAY_TYPE, "Dictionary"), "set_query_result", "get_query_result");

	ClassDB::bind_method(D_METHOD("get_query_result_by_reference"), &SQLite::get_query_result_by_reference);
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "query_result_by_reference", PROPERTY_HINT_ARRAY_TYPE, "Dictionary"), "set_query_result", "get_query_result_by_reference");

	// Constants.
	BIND_ENUM_CONSTANT(QUIET);
	BIND_ENUM_CONSTANT(NORMAL);
	BIND_ENUM_CONSTANT(VERBOSE);
	BIND_ENUM_CONSTANT(VERY_VERBOSE);

	BIND_CONSTANT(SQLITE_OK); /* Successful result */
	/* beginning-of-error-codes */
	BIND_CONSTANT(SQLITE_ERROR);
	BIND_CONSTANT(SQLITE_INTERNAL);
	BIND_CONSTANT(SQLITE_PERM);
	BIND_CONSTANT(SQLITE_ABORT);
	BIND_CONSTANT(SQLITE_BUSY);
	BIND_CONSTANT(SQLITE_LOCKED);
	BIND_CONSTANT(SQLITE_NOMEM);
	BIND_CONSTANT(SQLITE_READONLY);
	BIND_CONSTANT(SQLITE_INTERRUPT);
	BIND_CONSTANT(SQLITE_IOERR);
	BIND_CONSTANT(SQLITE_CORRUPT);
	BIND_CONSTANT(SQLITE_NOTFOUND);
	BIND_CONSTANT(SQLITE_FULL);
	BIND_CONSTANT(SQLITE_CANTOPEN);
	BIND_CONSTANT(SQLITE_PROTOCOL);
	BIND_CONSTANT(SQLITE_EMPTY);
	BIND_CONSTANT(SQLITE_SCHEMA);
	BIND_CONSTANT(SQLITE_TOOBIG);
	BIND_CONSTANT(SQLITE_CONSTRAINT);
	BIND_CONSTANT(SQLITE_MISMATCH);
	BIND_CONSTANT(SQLITE_MISUSE);
	BIND_CONSTANT(SQLITE_NOLFS);
	BIND_CONSTANT(SQLITE_AUTH);
	BIND_CONSTANT(SQLITE_FORMAT);
	BIND_CONSTANT(SQLITE_RANGE);
	BIND_CONSTANT(SQLITE_NOTADB);
	BIND_CONSTANT(SQLITE_NOTICE);
	BIND_CONSTANT(SQLITE_WARNING);
	BIND_CONSTANT(SQLITE_ROW);
	BIND_CONSTANT(SQLITE_DONE);
	/* end-of-error-codes */
}

SQLite::SQLite() {
	db = nullptr;
	query_result = TypedArray<Dictionary>();
}

SQLite::~SQLite() {
	/* Clean up the function_registry */
	function_registry.clear();
	function_registry.shrink_to_fit();
	/* Close the database connection if it is still open */
	if (db) {
		close_db();
	}
}

bool SQLite::open_db() {
	if (db) {
		ERR_PRINT("GDSQLite Error: Can't open database if connection is already open!");
		return false;
	}

	char *zErrMsg = 0;
	int rc;
	if (path.find(":memory:") == -1) {
		/* Add the default_extension to the database path if no extension is present */
		/* Skip if the default_extension is an empty string to allow for paths without extension */
		if (path.get_extension().is_empty() && !default_extension.is_empty()) {
			String ending = String(".") + default_extension;
			path += ending;
		}

		if (!read_only) {
			/* Find the real path */
			path = ProjectSettings::get_singleton()->globalize_path(path.strip_edges());
		}
	}

	// TODO: Switch back to the `alloc_c_string()`-method once the API gets updated
	const CharString dummy_path = path.utf8();
	const char *char_path = dummy_path.get_data();
	//const char *char_path = path.alloc_c_string();
	/* Try to open the database */
	if (read_only) {
		if (path.find(":memory:") == -1) {
			sqlite3_vfs_register(gdsqlite_vfs(), 0);
			rc = sqlite3_open_v2(char_path, &db, SQLITE_OPEN_READONLY, "godot");
		} else {
			ERR_PRINT("GDSQLite Error: Opening in-memory databases in read-only mode is currently not supported!");
			return false;
		}
	} else {
		/* The `SQLITE_OPEN_URI`-flag is solely required for in-memory databases with shared cache, but it is safe to use in most general cases */
		/* As discussed here: https://www.sqlite.org/uri.html */
		rc = sqlite3_open_v2(char_path, &db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL);
		/* Identical to: `rc = sqlite3_open(char_path, &db);`*/
	}

	if (rc != SQLITE_OK) {
		ERR_PRINT("GDSQLite Error: Can't open database: " + String::utf8(sqlite3_errmsg(db)));
		return false;
	} else if (verbosity_level > VerbosityLevel::QUIET) {
		UtilityFunctions::print("Opened database successfully (" + path + ")");
	}

	/* Try to enable foreign keys. */
	if (foreign_keys) {
		rc = sqlite3_exec(db, "PRAGMA foreign_keys=on;", NULL, NULL, &zErrMsg);
		if (rc != SQLITE_OK) {
			ERR_PRINT("GDSQLite Error: Can't enable foreign keys: " + String::utf8(zErrMsg));
			sqlite3_free(zErrMsg);
			return false;
		}
	}

	return true;
}

bool SQLite::close_db() {
	if (db) {
		// Cannot close database!
		if (sqlite3_close_v2(db) != SQLITE_OK) {
			ERR_PRINT("GDSQLite Error: Can't close database!");
			return false;
		} else {
			db = nullptr;
			if (verbosity_level > VerbosityLevel::QUIET) {
				UtilityFunctions::print("Closed database (" + path + ")");
			}
			return true;
		}
	}

	ERR_PRINT("GDSQLite Error: Can't close database if connection is not open!");
	return false;
}

bool SQLite::query(const String &p_query) {
	return query_with_bindings(p_query, Array());
}

bool SQLite::query_with_bindings(const String &p_query, Array param_bindings) {
	const char *zErrMsg, *sql, *pzTail;
	int rc;

	if (verbosity_level > VerbosityLevel::NORMAL) {
		UtilityFunctions::print(p_query);
	}
	// TODO: Switch back to the `alloc_c_string()`-method once the API gets updated
	const CharString dummy_query = p_query.utf8();
	sql = dummy_query.get_data();
	//sql = p_query.alloc_c_string();

	/* Clear the previous query results */
	query_result.clear();

	sqlite3_stmt *stmt;
	/* Prepare an SQL statement */
	rc = sqlite3_prepare_v2(db, sql, -1, &stmt, &pzTail);
	zErrMsg = sqlite3_errmsg(db);
	error_message = String::utf8(zErrMsg);
	if (rc != SQLITE_OK) {
		ERR_PRINT(" --> SQL error: " + error_message);
		sqlite3_finalize(stmt);
		return false;
	}

	/* Check if the param_bindings size exceeds the required parameter count */
	int parameter_count = sqlite3_bind_parameter_count(stmt);
	if (param_bindings.size() < parameter_count) {
		ERR_PRINT("GDSQLite Error: Insufficient number of parameters to satisfy required number of bindings in statement!");
		sqlite3_finalize(stmt);
		return false;
	}

	/* Bind any given parameters to the prepared statement */
	for (int i = 0; i < parameter_count; i++) {
		Variant binding_value = param_bindings.pop_front();
		switch (binding_value.get_type()) {
			case Variant::NIL:
				sqlite3_bind_null(stmt, i + 1);
				break;

			case Variant::BOOL:
			case Variant::INT:
				sqlite3_bind_int64(stmt, i + 1, int64_t(binding_value));
				break;

			case Variant::FLOAT:
				sqlite3_bind_double(stmt, i + 1, binding_value);
				break;

			case Variant::STRING:
				// TODO: Switch back to the `alloc_c_string()`-method once the API gets updated
				{
					const CharString dummy_binding = (binding_value.operator String()).utf8();
					const char *binding = dummy_binding.get_data();
					sqlite3_bind_text(stmt, i + 1, binding, -1, SQLITE_TRANSIENT);
				}
				//sqlite3_bind_text(stmt, i + 1, (binding_value.operator String()).alloc_c_string(), -1, SQLITE_TRANSIENT);
				break;

			case Variant::PACKED_BYTE_ARRAY: {
				PackedByteArray binding = ((const PackedByteArray &)binding_value);
				/* Calling .ptr() on an empty PackedByteArray returns an error */
				if (binding.size() == 0) {
					sqlite3_bind_null(stmt, i + 1);
					/* Identical to: `sqlite3_bind_blob64(stmt, i + 1, nullptr, 0, SQLITE_TRANSIENT);`*/
				} else {
					sqlite3_bind_blob64(stmt, i + 1, binding.ptr(), binding.size(), SQLITE_TRANSIENT);
				}
				break;
			}

			default:
				ERR_PRINT("GDSQLite Error: Binding a parameter of type " + String(std::to_string(binding_value.get_type()).c_str()) + " (TYPE_*) is not supported!");
				sqlite3_finalize(stmt);
				return false;
		}
	}

	if (verbosity_level > VerbosityLevel::NORMAL) {
		char *expanded_sql = sqlite3_expanded_sql(stmt);
		UtilityFunctions::print(String::utf8(expanded_sql));
		sqlite3_free(expanded_sql);
	}

	// Execute the statement and iterate over all the resulting rows.
	while (sqlite3_step(stmt) == SQLITE_ROW) {
		Dictionary column_dict;
		int argc = sqlite3_column_count(stmt);

		/* Loop over all columns and add them to the Dictionary */
		for (int i = 0; i < argc; i++) {
			Variant column_value;
			/* Check the column type and do correct casting */
			switch (sqlite3_column_type(stmt, i)) {
				case SQLITE_INTEGER:
					column_value = Variant((int64_t)sqlite3_column_int64(stmt, i));
					break;

				case SQLITE_FLOAT:
					column_value = Variant(sqlite3_column_double(stmt, i));
					break;

				case SQLITE_TEXT:
					column_value = Variant(String::utf8((char *)sqlite3_column_text(stmt, i)));
					break;

				case SQLITE_BLOB: {
					int bytes = sqlite3_column_bytes(stmt, i);
					PackedByteArray arr = PackedByteArray();
					arr.resize(bytes);
					memcpy((void *)arr.ptrw(), (char *)sqlite3_column_blob(stmt, i), bytes);
					column_value = arr;
					break;
				}

				case SQLITE_NULL:
					break;

				default:
					break;
			}

			const char *azColName = sqlite3_column_name(stmt, i);
			column_dict[String::utf8(azColName)] = column_value;
		}
		/* Add result to query_result Array */
		query_result.append(column_dict);
	}

	/* Clean up and delete the resources used by the prepared statement */
	sqlite3_finalize(stmt);

	rc = sqlite3_errcode(db);
	zErrMsg = sqlite3_errmsg(db);
	error_message = String::utf8(zErrMsg);
	if (rc != SQLITE_OK) {
		ERR_PRINT(" --> SQL error: " + error_message);
		return false;
	} else if (verbosity_level > VerbosityLevel::NORMAL) {
		UtilityFunctions::print(" --> Query succeeded");
	}

	/* Figure out if there's a subsequent statement which needs execution */
	String sTail = String::utf8(pzTail).strip_edges();
	if (!sTail.is_empty()) {
		return query_with_bindings(sTail, param_bindings);
	}

	if (!param_bindings.is_empty()) {
		WARN_PRINT("GDSQLite Warning: Provided number of bindings exceeded the required number in statement! (" + String(std::to_string(param_bindings.size()).c_str()) + " unused parameter(s))");
	}

	return true;
}

bool SQLite::create_table(const String &p_name, const Dictionary &p_table_dict) {
	if (!validate_table_dict(p_table_dict)) {
		return false;
	}

	String query_string, type_string, key_string, primary_string;
	String integer_datatype = "int";
	/* Create SQL statement */
	query_string = "CREATE TABLE IF NOT EXISTS " + p_name + " (";
	key_string = "";
	primary_string = "";

	Dictionary column_dict;
	Array columns = p_table_dict.keys();
	int64_t number_of_columns = columns.size();
	int primary_key_columns = 0;
	for (int64_t i = 0; i <= number_of_columns - 1; i++) {
		column_dict = p_table_dict[columns[i]];
		if (column_dict.get("primary_key", false)) {
			primary_key_columns++;
		}
	}
	for (int64_t i = 0; i <= number_of_columns - 1; i++) {
		column_dict = p_table_dict[columns[i]];
		query_string += (const String &)columns[i] + String(" ");
		type_string = (const String &)column_dict["data_type"];
		if (type_string.to_lower().begins_with(integer_datatype)) {
			query_string += String("INTEGER");
		} else {
			query_string += type_string;
		}

		/* Primary key check */
		if (column_dict.get("primary_key", false)) {
			if (primary_key_columns == 1) {
				query_string += String(" PRIMARY KEY");
				/* Autoincrement check */
				if (column_dict.get("auto_increment", false)) {
					query_string += String(" AUTOINCREMENT");
				}
			} else {
				if (primary_string.is_empty()) {
					primary_string = (const String &)columns[i];
				} else {
					primary_string += String(", ") + (const String &)columns[i];
				}
			}
		}
		/* Not null check */
		if (column_dict.get("not_null", false)) {
			query_string += String(" NOT NULL");
		}
		/* Unique check */
		if (column_dict.get("unique", false)) {
			query_string += String(" UNIQUE");
		}
		/* Default check */
		if (column_dict.has("default")) {
			query_string += String(" DEFAULT ") + (const String &)column_dict["default"];
		}
		/* Apply foreign key constraint. */
		if (foreign_keys) {
			if (column_dict.get("foreign_key", false)) {
				const String foreign_key_definition = (const String &)(column_dict["foreign_key"]);
				const Array foreign_key_elements = foreign_key_definition.split(".");
				if (foreign_key_elements.size() == 2) {
					const String column_name = (const String &)(columns[i]);
					const String foreign_key_table_name = (const String &)(foreign_key_elements[0]);
					const String foreign_key_column_name = (const String &)(foreign_key_elements[1]);
					key_string += String(", FOREIGN KEY (" + column_name + ") REFERENCES " + foreign_key_table_name + "(" + foreign_key_column_name + ")");

					if (column_dict.has("on_delete")) {
						const String foreign_key_on_delete_action = (const String &)(column_dict["on_delete"]);
						key_string += String(" ON DELETE " + foreign_key_on_delete_action);
					}

					if (column_dict.has("on_update")) {
						const String foreign_key_on_update_action = (const String &)(column_dict["on_update"]);
						key_string += String(" ON UPDATE " + foreign_key_on_update_action);
					}
				}
			}
		}

		if (i != number_of_columns - 1) {
			query_string += ",";
		}
	}

	if (primary_key_columns > 1) {
		query_string += String(", PRIMARY KEY (") + primary_string + String(")");
	}

	query_string += key_string + ");";

	return query(query_string);
}

bool SQLite::validate_table_dict(const Dictionary &p_table_dict) {
	Dictionary column_dict;
	Array columns = p_table_dict.keys();
	int64_t number_of_columns = columns.size();
	for (int64_t i = 0; i <= number_of_columns - 1; i++) {
		if (p_table_dict[columns[i]].get_type() != Variant::DICTIONARY) {
			ERR_PRINT("GDSQLite Error: All values of the table dictionary should be of type Dictionary");
			return false;
		}

		column_dict = p_table_dict[columns[i]];
		if (!column_dict.has("data_type")) {
			ERR_PRINT("GDSQLite Error: The field \"data_type\" is a required part of the table dictionary");
			return false;
		}

		if (column_dict["data_type"].get_type() != Variant::STRING) {
			ERR_PRINT("GDSQLite Error: The field \"data_type\" should be of type String");
			return false;
		}

		if (column_dict.has("default")) {
			Variant::Type default_type = column_dict["default"].get_type();

			CharString dummy_data_type = ((const String &)column_dict["data_type"]).utf8();
			const char *char_data_type = dummy_data_type.get_data();

			/* Get the type of the "datatype"-field and compare with the type of the "default"-value */
			/* Some types are not checked and might be added in a future version */
			Variant::Type data_type_type = default_type;
			if (strcmp(char_data_type, "int") == 0) {
				data_type_type = Variant::Type::INT;
			} else if (strcmp(char_data_type, "text") == 0) {
				data_type_type = Variant::Type::STRING;
			} else if (strcmp(char_data_type, "real") == 0) {
				data_type_type = Variant::Type::FLOAT;
			}

			if (data_type_type != default_type) {
				ERR_PRINT("GDSQLite Error: The type of the field \"default\" ( " + String(std::to_string(default_type).c_str()) + " ) should be the same type as the \"datatype\"-field ( " + String(std::to_string(data_type_type).c_str()) + " )");
				return false;
			}
		}
	}

	return true;
}

bool SQLite::drop_table(const String &p_name) {
	String query_string;
	/* Create SQL statement */
	query_string = "DROP TABLE " + p_name + ";";

	return query(query_string);
}

bool SQLite::backup_to(String destination_path) {
	destination_path = ProjectSettings::get_singleton()->globalize_path(destination_path.strip_edges());
	CharString dummy_path = destination_path.utf8();
	const char *char_path = dummy_path.get_data();

	sqlite3 *destination_db;
	int result = sqlite3_open_v2(char_path, &destination_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL);
	if (result == SQLITE_OK) {
		result = backup_database(db, destination_db);
	}
	(void)sqlite3_close_v2(destination_db);
	return result == SQLITE_OK;
}

bool SQLite::restore_from(String source_path) {
	source_path = ProjectSettings::get_singleton()->globalize_path(source_path.strip_edges());
	CharString dummy_path = source_path.utf8();
	const char *char_path = dummy_path.get_data();

	sqlite3 *source_db;
	int result = sqlite3_open_v2(char_path, &source_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_URI, NULL);
	if (result == SQLITE_OK) {
		result = backup_database(source_db, db);
	}
	(void)sqlite3_close_v2(source_db);
	return result == SQLITE_OK;
}

int SQLite::backup_database(sqlite3 *source_db, sqlite3 *destination_db) {
	int rc;
	sqlite3_backup *backup = sqlite3_backup_init(destination_db, "main", source_db, "main");
	if (backup) {
		(void)sqlite3_backup_step(backup, -1);
		(void)sqlite3_backup_finish(backup);
	}

	rc = sqlite3_errcode(destination_db);
	return rc;
}

bool SQLite::insert_row(const String &p_name, const Dictionary &p_row_dict) {
	String query_string, key_string, value_string = "";
	Array keys = p_row_dict.keys();
	Array param_bindings = p_row_dict.values();

	/* Create SQL statement */
	query_string = "INSERT INTO " + p_name;

	int64_t number_of_keys = p_row_dict.size();
	for (int64_t i = 0; i <= number_of_keys - 1; i++) {
		key_string += (const String &)keys[i];
		value_string += "?";
		if (i != number_of_keys - 1) {
			key_string += ",";
			value_string += ",";
		}
	}
	query_string += " (" + key_string + ") VALUES (" + value_string + ");";

	return query_with_bindings(query_string, param_bindings);
}

bool SQLite::insert_rows(const String &p_name, const Array &p_row_array) {
	query("BEGIN TRANSACTION;");
	int64_t number_of_rows = p_row_array.size();
	for (int64_t i = 0; i <= number_of_rows - 1; i++) {
		if (p_row_array[i].get_type() != Variant::DICTIONARY) {
			ERR_PRINT("GDSQLite Error: All elements of the Array should be of type Dictionary");
			/* Don't forget to close the transaction! */
			/* Maybe we should do a rollback instead? */
			query("END TRANSACTION;");
			return false;
		}
		if (!insert_row(p_name, p_row_array[i])) {
			/* Don't forget to close the transaction! */
			/* Stop the error_message from being overwritten! */
			String previous_error_message = error_message;
			query("END TRANSACTION;");
			error_message = previous_error_message;
			return false;
		}
	}
	query("END TRANSACTION;");
	return true;
}

Array SQLite::select_rows(const String &p_name, const String &p_conditions, const Array &p_columns_array) {
	String query_string;
	/* Create SQL statement */
	query_string = "SELECT ";

	int64_t number_of_columns = p_columns_array.size();
	for (int64_t i = 0; i <= number_of_columns - 1; i++) {
		if (p_columns_array[i].get_type() != Variant::STRING) {
			ERR_PRINT("GDSQLite Error: All elements of the Array should be of type String");
			return query_result;
		}
		query_string += (const String &)p_columns_array[i];

		if (i != number_of_columns - 1) {
			query_string += ", ";
		}
	}
	query_string += " FROM " + p_name;
	if (!p_conditions.is_empty()) {
		query_string += " WHERE " + p_conditions;
	}
	query_string += ";";

	query(query_string);
	/* Return the duplicated result */
	return get_query_result();
}

bool SQLite::update_rows(const String &p_name, const String &p_conditions, const Dictionary &p_updated_row_dict) {
	String query_string;
	Array param_bindings;
	bool success;

	int64_t number_of_keys = p_updated_row_dict.size();
	Array keys = p_updated_row_dict.keys();
	Array values = p_updated_row_dict.values();

	query("BEGIN TRANSACTION;");
	/* Create SQL statement */
	query_string += "UPDATE " + p_name + " SET ";

	for (int64_t i = 0; i <= number_of_keys - 1; i++) {
		query_string += (const String &)keys[i] + String("=?");
		param_bindings.append(values[i]);
		if (i != number_of_keys - 1) {
			query_string += ", ";
		}
	}
	query_string += " WHERE " + p_conditions + ";";

	success = query_with_bindings(query_string, param_bindings);
	/* Stop the error_message from being overwritten! */
	String previous_error_message = error_message;
	query("END TRANSACTION;");
	error_message = previous_error_message;
	return success;
}

bool SQLite::delete_rows(const String &p_name, const String &p_conditions) {
	String query_string;
	bool success;

	query("BEGIN TRANSACTION;");
	/* Create SQL statement */
	query_string = "DELETE FROM " + p_name;
	/* If it's empty or * everything is to be deleted */
	if (!p_conditions.is_empty() && (p_conditions != (const String &)"*")) {
		query_string += " WHERE " + p_conditions;
	}
	query_string += ";";

	success = query(query_string);
	/* Stop the error_message from being overwritten! */
	String previous_error_message = error_message;
	query("END TRANSACTION;");
	error_message = previous_error_message;
	return success;
}

static void function_callback(sqlite3_context *context, int argc, sqlite3_value **argv) {
	void *temp = sqlite3_user_data(context);
	Callable callable = *(Callable *)temp;

	/* Check if the callable is valid */
	if (!callable.is_valid()) {
		ERR_PRINT("GDSQLite Error: Supplied function reference is invalid! Aborting callback...");
		return;
	}

	Array argument_array = Array();
	Variant argument_value;
	for (int i = 0; i <= argc - 1; i++) {
		sqlite3_value *value = *argv;
		/* Check the value type and do correct casting */
		switch (sqlite3_value_type(value)) {
			case SQLITE_INTEGER:
				argument_value = Variant((int64_t)sqlite3_value_int64(value));
				break;

			case SQLITE_FLOAT:
				argument_value = Variant(sqlite3_value_double(value));
				break;

			case SQLITE_TEXT:
				argument_value = Variant((char *)sqlite3_value_text(value));
				break;

			case SQLITE_BLOB: {
				int bytes = sqlite3_value_bytes(value);
				PackedByteArray arr = PackedByteArray();
				arr.resize(bytes);
				memcpy((void *)arr.ptrw(), (char *)sqlite3_value_blob(value), bytes);
				argument_value = arr;
				break;
			}

			case SQLITE_NULL:
				break;

			default:
				break;
		}

		argument_array.append(argument_value);
		argv += 1;
	}

	Variant output = callable.callv(argument_array);

	switch (output.get_type()) {
		case Variant::NIL:
			sqlite3_result_null(context);
			break;

		case Variant::BOOL:
		case Variant::INT:
			sqlite3_result_int64(context, int64_t(output));
			break;

		case Variant::FLOAT:
			sqlite3_result_double(context, output);
			break;

		case Variant::STRING:
			// TODO: Switch back to the `alloc_c_string()`-method once the API gets updated
			{
				const CharString dummy_binding = (output.operator String()).utf8();
				const char *binding = dummy_binding.get_data();
				sqlite3_result_text(context, binding, -1, SQLITE_STATIC);
			}
			//sqlite3_result_text(context, (output.operator String()).alloc_c_string(), -1, SQLITE_STATIC);
			break;

		case Variant::PACKED_BYTE_ARRAY: {
			PackedByteArray arr = ((const PackedByteArray &)output);
			sqlite3_result_blob(context, arr.ptr(), arr.size(), SQLITE_TRANSIENT);
			break;
		}

		default:
			break;
	}
}

bool SQLite::create_function(const String &p_name, const Callable &p_callable, int p_argc) {
	/* The exact memory position of the std::vector's elements changes during memory reallocation (= when adding additional elements) */
	/* Luckily, the pointer to the managed object (of the std::unique_ptr) won't change during execution! (= consistent) */
	/* The std::unique_ptr is stored in a std::vector and is thus allocated on the heap */
	function_registry.push_back(std::make_unique<Callable>(p_callable));

	int rc;
	CharString dummy_name = p_name.utf8();
	const char *zFunctionName = dummy_name.get_data();
	//const char *zFunctionName = p_name.alloc_c_string();
	int nArg = p_argc;
	int eTextRep = SQLITE_UTF8;

	/* Get a void pointer to the managed object of the smart pointer that is stored at the back of the vector */
	void *pApp = (void *)function_registry.back().get();
	void (*xFunc)(sqlite3_context *, int, sqlite3_value **) = function_callback;
	void (*xStep)(sqlite3_context *, int, sqlite3_value **) = NULL;
	void (*xFinal)(sqlite3_context *) = NULL;

	/* Create the actual function */
	rc = sqlite3_create_function(db, zFunctionName, nArg, eTextRep, pApp, xFunc, xStep, xFinal);
	if (rc) {
		ERR_PRINT("GDSQLite Error: " + String(sqlite3_errmsg(db)));
		return false;
	} else if (verbosity_level > VerbosityLevel::NORMAL) {
		UtilityFunctions::print("Succesfully added function \"" + p_name + "\" to function registry");
	}
	return true;
}

bool SQLite::import_from_json(String import_path) {
	/* Add .json to the import_path String if not present */
	String ending = String(".json");
	if (!import_path.ends_with(ending)) {
		import_path += ending;
	}
	/* Find the real path */
	import_path = ProjectSettings::get_singleton()->globalize_path(import_path.strip_edges());
	CharString dummy_path = import_path.utf8();
	const char *char_path = dummy_path.get_data();
	//const char *char_path = import_path.alloc_c_string();

	/* Open the json-file and stream its content into a stringstream */
	std::ifstream ifs(char_path);
	if (ifs.fail()) {
		ERR_PRINT("GDSQLite Error: " + String(std::strerror(errno)) + " (" + import_path + ")");
		return false;
	}

	std::stringstream buffer;
	buffer << ifs.rdbuf();
	std::string str = buffer.str();
	ifs.close();

	String json_string = String::utf8(str.c_str());
	PackedByteArray json_buffer = json_string.to_utf8_buffer();

	return import_from_buffer(json_buffer);
}

bool SQLite::export_to_json(String export_path) {
	PackedByteArray json_buffer = export_to_buffer();
	String json_string = json_buffer.get_string_from_utf8();

	/* Add .json to the import_path String if not present */
	String ending = String(".json");
	if (!export_path.ends_with(ending)) {
		export_path += ending;
	}
	/* Find the real path */
	export_path = ProjectSettings::get_singleton()->globalize_path(export_path.strip_edges());
	CharString dummy_path = export_path.utf8();
	const char *char_path = dummy_path.get_data();
	//const char *char_path = export_path.alloc_c_string();

	std::ofstream ofs(char_path, std::ios::trunc);
	if (ofs.fail()) {
		ERR_PRINT("GDSQLite Error: " + String(std::strerror(errno)) + " (" + export_path + ")");
		return false;
	}

	CharString dummy_string = json_string.utf8();
	ofs << dummy_string.get_data();
	//ofs << json_string.alloc_c_string();
	ofs.close();

	return true;
}

bool SQLite::import_from_buffer(PackedByteArray json_buffer) {
	/* Attempt to parse the input json_string and, if unsuccessful, throw a parse error specifying the erroneous line */

	Ref<JSON> json;
	json.instantiate();
	String json_string = json_buffer.get_string_from_utf8();
	Error error = json->parse(json_string);
	if (error != Error::OK) {
		/* Throw a parsing error */
		ERR_PRINT("GDSQLite Error: parsing failed! reason: " + json->get_error_message() + ", at line: " + String::num_int64(json->get_error_line()));
		return false;
	}
	Array database_array = json->get_data();
	std::vector<object_struct> objects_to_import;
	/* Validate the json structure and populate the tables_to_import vector */
	if (!validate_json(database_array, objects_to_import)) {
		return false;
	}

	/* Check if the database is open and, if not, attempt to open it */
	if (db == nullptr) {
		/* Open the database using the open_db method */
		if (!open_db()) {
			return false;
		}
	}

	/* Find all views that are present in this database */
	query(String("SELECT name FROM sqlite_master WHERE type = 'view';"));
	TypedArray<Dictionary> old_view_array = query_result.duplicate(true);
	int64_t old_number_of_views = old_view_array.size();
	/* Drop all old views present in the database */
	for (int64_t i = 0; i <= old_number_of_views - 1; i++) {
		Dictionary view_dict = old_view_array[i];
		String view_name = view_dict["name"];
		String query_string = "DROP VIEW " + view_name + ";";

		query(query_string);
	}

	/* Find all tables that are present in this database */
	/* We don't care about indexes or triggers here since they get dropped automatically when their table is dropped */
	query(String("SELECT name FROM sqlite_master WHERE type = 'table';"));
	TypedArray<Dictionary> old_table_array = query_result.duplicate(true);
#ifdef SQLITE_ENABLE_FTS5
	/* FTS5 creates a bunch of shadow tables that cannot be dropped manually! */
	/* The virtual table is responsible for dropping these tables itself */
	remove_shadow_tables(old_table_array);
#endif
	int64_t old_number_of_tables = old_table_array.size();
	/* Drop all old tables present in the database */
	for (int64_t i = 0; i <= old_number_of_tables - 1; i++) {
		Dictionary table_dict = old_table_array[i];
		String table_name = table_dict["name"];

		drop_table(table_name);
	}

	query("BEGIN TRANSACTION;");
	/* foreign_keys cannot be enforced until after all rows have been added! */
	query("PRAGMA defer_foreign_keys=on;");
	/* Add all new tables and fill them up with all the rows */
	for (auto table : objects_to_import) {
		if (!query(table.sql)) {
			/* Don't forget to close the transaction! */
			/* Stop the error_message from being overwritten! */
			String previous_error_message = error_message;
			query("END TRANSACTION;");
			error_message = previous_error_message;
		}
	}

	for (auto object : objects_to_import) {
		if (object.type != TABLE) {
			/* The object is an index, view or trigger and doesn't have any rows! */
			continue;
		}

		/* Convert the base64-encoded columns back to raw data */
		for (int64_t i = 0; i <= object.base64_columns.size() - 1; i++) {
			String key = object.base64_columns[i];
			for (int64_t j = 0; j <= object.row_array.size() - 1; j++) {
				Dictionary row = object.row_array[j];

				if (row.has(key)) {
					String encoded_string = ((const String &)row[key]);
					PackedByteArray arr = Marshalls::get_singleton()->base64_to_raw(encoded_string);
					row[key] = arr;
				}
			}
		}

		int64_t number_of_rows = object.row_array.size();
		for (int64_t i = 0; i <= number_of_rows - 1; i++) {
			if (object.row_array[i].get_type() != Variant::DICTIONARY) {
				ERR_PRINT("GDSQLite Error: All elements of the Array should be of type Dictionary");
				return false;
			}
			if (!insert_row(object.name, object.row_array[i])) {
				/* Don't forget to close the transaction! */
				/* Stop the error_message from being overwritten! */
				String previous_error_message = error_message;
				query("END TRANSACTION;");
				error_message = previous_error_message;
				return false;
			}
		}
	}
	query("END TRANSACTION;");
	return true;
}

PackedByteArray SQLite::export_to_buffer() {
	/* Get all names and sql templates for all tables present in the database */
	query(String("SELECT name,sql,type FROM sqlite_master;"));
	TypedArray<Dictionary> database_array = query_result.duplicate(true);
#ifdef SQLITE_ENABLE_FTS5
	/* FTS5 creates a bunch of shadow tables that should NOT be exported! */
	remove_shadow_tables(database_array);
#endif
	int64_t number_of_objects = database_array.size();
	/* Construct a Dictionary for each table, convert it to JSON and write it to String */
	for (int64_t i = 0; i <= number_of_objects - 1; i++) {
		Dictionary object_dict = database_array[i];

		if (object_dict["type"] == String("table")) {
			String object_name = object_dict["name"];
			String query_string;

			query_string = "SELECT * FROM " + (const String &)object_name + ";";
			query(query_string);

			/* Encode all columns of type PoolByteArray to base64 */
			if (!query_result.is_empty()) {
				/* First identify the columns that are of this type! */
				Array base64_columns = Array();
				Dictionary initial_row = query_result[0];
				Array keys = initial_row.keys();
				for (int k = 0; k <= keys.size() - 1; k++) {
					String key = keys[k];
					Variant value = initial_row[key];
					if (value.get_type() == Variant::PACKED_BYTE_ARRAY) {
						base64_columns.append(key);
					}
				}

				/* Now go through all the rows and encode the relevant columns */
				for (int k = 0; k <= base64_columns.size() - 1; k++) {
					String key = base64_columns[k];
					for (int j = 0; j <= query_result.size() - 1; j++) {
						Dictionary row = query_result[j];
						PackedByteArray arr = ((const PackedByteArray &)row[key]);
						String encoded_string = Marshalls::get_singleton()->raw_to_base64(arr);

						row.erase(key);
						row[key] = encoded_string;
					}
				}

				if (!base64_columns.is_empty()) {
					object_dict["base64_columns"] = base64_columns;
				}
			}
			object_dict["row_array"] = query_result.duplicate(true);
		}
	}

	Ref<JSON> json;
	json.instantiate();
	String json_string = json->stringify(database_array, "\t");
	PackedByteArray json_buffer = json_string.to_utf8_buffer();
	return json_buffer;
}

bool SQLite::validate_json(const Array &database_array, std::vector<object_struct> &objects_to_import) {
	/* Start going through all the tables and checking their validity */
	int64_t number_of_objects = database_array.size();
	for (int64_t i = 0; i <= number_of_objects - 1; i++) {
		/* Create a new object_struct */
		object_struct new_object;

		Dictionary temp_dict = database_array[i];
		/* Get the name of the object */
		if (!temp_dict.has("name")) {
			/* Did not find the necessary key! */
			ERR_PRINT("GDSQlite Error: Did not find required key \"name\" in the supplied json-file");
			return false;
		}
		new_object.name = temp_dict["name"];

		/* Extract the sql template for generating the object */
		if (!temp_dict.has("sql")) {
			/* Did not find the necessary key! */
			ERR_PRINT("GDSQlite Error: Did not find required key \"sql\" in the supplied json-file");
			return false;
		}
		new_object.sql = temp_dict["sql"];

		if (!temp_dict.has("type")) {
			/* Did not find the necessary key! */
			ERR_PRINT("GDSQlite Error: Did not find required key \"type\" in the supplied json-file");
			return false;
		}
		if (temp_dict["type"] == String("table")) {
			new_object.type = TABLE;

			/* Add the optional base64_columns if defined! */
			new_object.base64_columns = temp_dict.get("base64_columns", Array());

			if (!temp_dict.has("row_array")) {
				/* Did not find the necessary key! */
				ERR_PRINT("GDSQlite Error: Did not find required key \"row_array\" in the supplied json-file");
				return false;
			} else if (Variant(temp_dict["row_array"]).get_type() != Variant::ARRAY) {
				ERR_PRINT("GDSQlite Error: The value of the key \"row_array\" should consist of an array of rows");
				return false;
			}
			new_object.row_array = temp_dict["row_array"];
		} else if (temp_dict["type"] == String("index")) {
			new_object.type = INDEX;
		} else if (temp_dict["type"] == String("view")) {
			new_object.type = TRIGGER;
		} else if (temp_dict["type"] == String("trigger")) {
			new_object.type = TRIGGER;
		} else {
			/* Did not find the necessary key! */
			ERR_PRINT("GDSQlite Error: The value of key \"type\" is restricted to \"table\", \"index\", \"view\" or \"trigger\"");
			return false;
		}

		/* Add the table or trigger to the new objects vector */
		objects_to_import.insert(objects_to_import.end(), new_object);
	}
	return true;
}

void SQLite::remove_shadow_tables(Array &p_array) {
	/* The rootpage of virtual tables is always zero!*/
	query(String("SELECT name FROM sqlite_master WHERE type = 'table' AND rootpage = 0;"));
	int number_of_objects = query_result.size();
	Array database_array = query_result.duplicate(true);

	/* Make an array of all the expected shadow table names */
	Array shadow_table_names = Array();
	for (int i = 0; i <= number_of_objects - 1; i++) {
		Dictionary object_dict = database_array[i];
		String virtual_table_name = object_dict["name"];

		/* These are the shadow tables created by FTS5, as discussed here: */
		/* https://www.sqlite.org/fts5.html */
		shadow_table_names.append(virtual_table_name + "_config");
		shadow_table_names.append(virtual_table_name + "_content");
		shadow_table_names.append(virtual_table_name + "_data");
		shadow_table_names.append(virtual_table_name + "_docsize");
		shadow_table_names.append(virtual_table_name + "_idx");
	}

	/* Get rid of all the shadow tables */
	Array clean_array = Array();
	for (int i = 0; i <= p_array.size() - 1; i++) {
		Dictionary object_dict = p_array[i];
		if (object_dict["type"] == String("table")) {
			String table_name = object_dict["name"];
			if (!shadow_table_names.has(table_name)) {
				clean_array.append(object_dict);
			}
		} else if (object_dict["type"] == String("trigger")) {
			clean_array.append(object_dict);
		}
	}

	p_array = clean_array;
}

// Properties.
void SQLite::set_last_insert_rowid(const int64_t &p_last_insert_rowid) {
	if (db) {
		sqlite3_set_last_insert_rowid(db, p_last_insert_rowid);
	}
}

int64_t SQLite::get_last_insert_rowid() const {
	if (db) {
		return sqlite3_last_insert_rowid(db);
	}
	/* Return the default value */
	return 0;
}

void SQLite::set_verbosity_level(const int64_t &p_verbosity_level) {
	verbosity_level = p_verbosity_level;
}

int64_t SQLite::get_verbosity_level() const {
	return verbosity_level;
}

void SQLite::set_foreign_keys(const bool &p_foreign_keys) {
	foreign_keys = p_foreign_keys;
}

bool SQLite::get_foreign_keys() const {
	return foreign_keys;
}

void SQLite::set_read_only(const bool &p_read_only) {
	read_only = p_read_only;
}

bool SQLite::get_read_only() const {
	return read_only;
}

void SQLite::set_path(const String &p_path) {
	path = p_path;
}

String SQLite::get_path() const {
	return path;
}

void SQLite::set_error_message(const String &p_error_message) {
	error_message = p_error_message;
}

String SQLite::get_error_message() const {
	return error_message;
}

void SQLite::set_default_extension(const String &p_default_extension) {
	default_extension = p_default_extension;
}

String SQLite::get_default_extension() const {
	return default_extension;
}

void SQLite::set_query_result(const TypedArray<Dictionary> &p_query_result) {
	query_result = p_query_result;
}

TypedArray<Dictionary> SQLite::get_query_result() const {
	return query_result.duplicate(true);
}

TypedArray<Dictionary> SQLite::get_query_result_by_reference() const {
	return query_result;
}

int SQLite::get_autocommit() const {
	if (db) {
		return sqlite3_get_autocommit(db);
	}
	/* Return the default value */
	return 1; // A non-zero value indicates the autocommit is on!
}

int SQLite::compileoption_used(const String &option_name) const {
	const CharString dummy_name = option_name.utf8();
	const char *char_name = dummy_name.get_data();
	return sqlite3_compileoption_used(char_name);
}

int SQLite::enable_load_extension(const bool &p_onoff) {
	int rc;
	if (p_onoff == true) {
		rc = sqlite3_enable_load_extension(db, 1);
	} else {
		rc = sqlite3_enable_load_extension(db, 0);
	}
	if (rc != SQLITE_OK) {
		ERR_PRINT("GDSQLite Error: Extension loading cannot be enabled/disabled.");
	}
	return rc;
}

int SQLite::load_extension(const String &p_path, const String &entrypoint) {
	int rc;

	char *zErrMsg = 0;
	String globalized_path = ProjectSettings::get_singleton()->globalize_path(p_path.strip_edges());
	const CharString dummy_path = globalized_path.utf8();
	const char *char_path = dummy_path.get_data();

	const CharString dummy_func_name = entrypoint.utf8();
	const char *func_name = dummy_func_name.get_data();

	// Enable C-API only
	sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 1, NULL);
	// load extension
	rc = sqlite3_load_extension(db, char_path, func_name, &zErrMsg);
	// Disable C-API
	sqlite3_db_config(db, SQLITE_DBCONFIG_ENABLE_LOAD_EXTENSION, 0, NULL);

	if (rc != SQLITE_OK) {
		ERR_PRINT("GDSQLite Error: Unable to load extension: " + String::utf8(zErrMsg));
		sqlite3_free(zErrMsg);
		return rc;
	}
	if (verbosity_level > VerbosityLevel::QUIET) {
		UtilityFunctions::print("Loaded extension successfully (" + globalized_path + ")");
	}

	return rc;
}