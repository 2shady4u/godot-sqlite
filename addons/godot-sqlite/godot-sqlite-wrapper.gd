class_name SQLiteWrapper
# This class aims to make documentation accessible in the Godot editor and
# is meant to be fully interchangeable with `gdsqlite.gdns`.
# More extensive documentation can be found here:
# https://github.com/2shady4u/godot-sqlite/blob/master/README.md

var SQLite = preload("res://addons/godot-sqlite/bin/gdsqlite.gdns")
var db

enum VerbosityLevel {
	QUIET = 0,
	NORMAL = 1,
	VERBOSE = 2,
	VERY_VERBOSE = 3
}

func _init():
	db = SQLite.new()

# VARIABLES --------------------------------------------------------------------

# NAME: path
# DEFAULT: ""
# DESCRIPTION:
# Path to the database, should be set before opening the database with `open_db()`.
var path : String setget set_path, get_path
func set_path(value : String) -> void:
	if db: db.path = value

func get_path() -> String:
	if db: return db.path
	return ""

# NAME: error_message
# DEFAULT: ""
# DESCRIPTION:
# Contains the zErrMsg returned by the SQLite query in human-readable form.
var error_message : String setget set_error_message, get_error_message
func set_error_message(message: String) -> void:
	if db: db.error_message = message

func get_error_message() -> String:
	if db: return db.error_message
	return ""

# NAME: default_extension
# DEFAULT: "db"
# DESCRIPTION:
# Default extension that is automatically appended to the `path`-variable whenever
# **no** extension is detected or given.
var default_extension :String setget set_default_extension, get_default_extension
func set_default_extension(value : String) -> void:
	if db: db.default_extension = value

func get_default_extension() -> String:
	if db: return db.default_extension
	return ""

# NAME: foreign_keys
# DEFAULT: false
# DESCRIPTION:
# Enables or disables the availability of foreign keys in the SQLite database.
var foreign_keys : bool setget set_foreign_keys, get_foreign_keys
func set_foreign_keys(value : bool) -> void:
	if db: db.foreign_keys = value

func get_foreign_keys() -> bool:
	if db: return db.foreign_keys
	return false

# NAME: read_only
# DEFAULT: false
# DESCRIPTION:
# Enabling this property opens the database in read-only mode & allows databases
# to be packaged inside of the PCK.
var read_only : bool setget set_read_only, get_read_only
func set_read_only(value : bool) -> void:
	if db: db.read_only = value

func get_read_only() -> bool:
	if db: return db.read_only
	return false

# NAME: query_result
# DEFAULT: []
# DESCRIPTION:
# Contains the results from the latest query by value; meaning that this property
# is safe to use when looping successive queries as it does not get overwritten by any future queries.
var query_result : Array setget set_query_result, get_query_result
func set_query_result(value : Array) -> void:
	if db: db.query_result = value

func get_query_result() -> Array:
	if db: return db.query_result
	return []

# NAME: query_result_by_reference
# DEFAULT: []
# DESCRIPTION:
# Contains the results from the latest query **by reference** and is, as a direct result,
# cleared and repopulated after every new query.
var query_result_by_reference : Array setget set_query_result_by_reference, get_query_result_by_reference
func set_query_result_by_reference(value : Array) -> void:
	if db: db.query_result_by_reference = value

func get_query_result_by_reference() -> Array:
	if db: return db.query_result_by_reference
	return []

# NAME: last_insert_rowid
# DEFAULT: -1
# DESCRIPTION:
# Exposes both the `sqlite3_last_insert_rowid()`- and `sqlite3_set_last_insert_rowid()`-methods to Godot as described [here](https://www.sqlite.org/c3ref/last_insert_rowid.html) and [here](https://www.sqlite.org/c3ref/set_last_insert_rowid.html) respectively.
var last_insert_rowid : int setget set_last_insert_rowid, get_last_insert_rowid
func set_last_insert_rowid(value : int) -> void:
	if db: db.last_insert_rowid = value

func get_last_insert_rowid() -> int:
	if !db: return db.last_insert_rowid
	return -1

# NAME: verbosity_level
# DEFAULT: -1
# DESCRIPTION:
# The verbosity_level determines the amount of logging to the Godot console that is handy for debugging your (possibly faulty) SQLite queries.
# Following levels are available:
# QUIET (0)       : Don't print anything to the console
# NORMAL (1)      : Print essential information to the console
# VERBOSE (2)     : Print additional information to the console
# VERY_VERBOSE (3): Same as VERBOSE
# **NOTE:** VERBOSE and higher levels might considerably slow down your queries due to excessive logging.
var verbosity_level : int setget set_verbosity_level, get_verbosity_level
func set_verbosity_level(var value) -> void:
	if db: db.verbosity_level = value

func get_verbosity_level() -> int:
	if db: return db.verbosity_level
	return -1

# FUNCTIONS --------------------------------------------------------------------

# Open a connection to the database at the path set by the `path`-variable.
# **NOTE:** The `path`-variable has to be set beforehand!
func open_db() -> bool:
	return db.open_db()

# Close the connection to the database.
func close_db() -> void:
	db.close_db()

# Executes a query on the database connection.
# **NOTE:** Allowing the user to directly access this method makes you vulnerable to SQL injection attacks!
func query(query_string : String) -> bool:
	return db.query(query_string)

# Executes a query with bindings on the database connection.
# For example, calling `query_with_bindings("SELECT name from monsters WHERE health < ?", [24])`
# would select all monsters with health below 24.
func query_with_bindings(query_string : String, param_bindings : Array) -> bool:
	return db.query_with_bindings(query_string, param_bindings)

# Creates the table with the name "table_name" if it doesn't already exist.
# For more information regarding the composition of the `table_dictionary`-property see:
# https://github.com/2shady4u/godot-sqlite/blob/master/README.md#functions
func create_table(table_name: String, table_dictionary: Dictionary) -> bool:
	return db.create_table(table_name, table_dictionary)

# Drops (removes) the table with the name "table_name" from the database.
func drop_table(table_name: String) -> bool:
	return db.drop_table(table_name)

# Insert a new row into the table with the given properties.
# For example, calling `insert_row("monsters", {"name": "Wolf", "health": 20})`
# would add a new row to the "monsters" table with the given properties.
func insert_row(table_name: String, row_dictionary : Dictionary) -> bool:
	return db.insert_row(table_name, row_dictionary)

# Insert multiple rows into the table with the given properties.
# For example, calling `insert_rows("monsters", [{"name": "Wolf", "health": 20}, {"name": "Boar", "health": 12}])`
# would add 2 new rows to the "monsters" table with the given properties.
func insert_rows(table_name: String, row_array : Array) -> bool:
	return db.insert_rows(table_name,row_array)

# Select all rows from the table that match the given conditions and return the wanted columns.
# For example, calling `select_rows("monsters", "health < 24", ["name", "health"])`
# would select all monsters with health below 24 and return their names & health in an array of dictionaries.
func select_rows(table_name : String, query_conditions: String, selected_columns: Array) -> Array:
	return db.select_rows(table_name,query_conditions,selected_columns)

# Update all rows from the table that match the given conditions.
# For example, calling `update_rows("monsters", "name = 'Wolf'", {"health": 24})`
# would update/change the health of all monsters with the name "Wolf" to 24.
func update_rows(table_name : String, query_conditions: String, updated_row_dictionary: Dictionary) -> bool:
	return db.update_rows(table_name, query_conditions, updated_row_dictionary)

# Delete all the rows in the table that match the given conditions.
# For example, calling `delete_rows("monsters", "name = 'Wolf' AND health > 18")`
# would delete all the rows in the "monsters" table that match the following conditions:
# - A "name" column value equal to "Wolf"
# - A "health" column value that is greater than 18
func delete_rows(table_name: String, query_conditions: String) -> bool:
	return db.delete_rows(table_name, query_conditions)

# Drops all database tables and imports the database structure and content present inside of `import_path.json`.
func import_from_json(import_path: String) -> bool:
	return db.import_from_json(import_path)

# Exports the database structure and content to `export_path.json` as a backup or for ease of editing.
func export_to_json(export_path : String) -> bool:
	return db.export_to_json(export_path)

# Bind a scalar SQL function to the database that can then be used in subsequent queries.
# Creating a function should only be seen as a measure of last resort and only be used when you perfectly know what you are doing.
func create_function(function_name: String, function_reference: FuncRef, number_of_arguments: int) -> bool:
	return db.create_function(function_name, function_reference, number_of_arguments)

# Get the current autocommit mode of the open database connection which can be used to check if
# there's any database transactions in progress, see here:
# http://www.sqlite.org/c3ref/get_autocommit.html
func get_autocommit() -> int:
	return db.get_autocommit()
