extends Node

const SQLite = preload("res://addons/godot-sqlite/bin/gdsqlite.gdns")
var db

var db_name := "res://data/test"

var table_name := "company"

var ids := [1,2,3,4,5,6,7]
var names := ["Paul","Allen","Teddy","Mark","Robert","Julia","Amanda"]
var ages := [32,25,23,25,30,63,13]
var addresses := ["California","Texas","Baltimore","Richmond","Texas","Atlanta","New-York"]
var salaries := [20000.00,15000.00,20000.00,65000.00,65000.00,65000.00,65000.00]

signal output_received(text)

func _ready():
	if OS.get_name() in ["Android", "iOS", "HTML5"]:
		copy_data_to_user()
		db_name = "user://data/test"

	# Enable/disable examples here:
	example_of_basic_database_querying()

func cprint(text : String) -> void:
	print(text)
	emit_signal("output_received", text)

func copy_data_to_user() -> void:
	var data_path := "res://data"
	var copy_path := "user://data"

	var dir = Directory.new()
	dir.make_dir(copy_path)
	if dir.open(data_path) == OK:
		dir.list_dir_begin();
		var file_name = dir.get_next()
		while (file_name != ""):
			if dir.current_is_dir():
				pass
			else:
				cprint("Copying " + file_name + " to /user-folder")
				dir.copy(data_path + "/" + file_name, copy_path + "/" + file_name)
			file_name = dir.get_next()
	else:
		cprint("An error occurred when trying to access the path.")

# Basic example that goes over all the basic features available in the addon, such
# as creating and dropping tables, inserting and deleting rows and doing more elementary
# PRAGMA queries.
func example_of_basic_database_querying():

	# Make a big table containing the variable types.
	var table_dict : Dictionary = Dictionary()
	table_dict["id"] = {"data_type":"int", "primary_key": true, "not_null": true}
	table_dict["name"] = {"data_type":"text", "not_null": true}
	table_dict["age"] = {"data_type":"int", "not_null": true}
	table_dict["address"] = {"data_type":"char(50)"}
	table_dict["salary"] = {"data_type":"real"}

	db = SQLite.new()
	db.path = db_name
	db.verbose_mode = true
	db.read_only = false
	# Open the database using the db_name found in the path variable
	db.open_db()

	if not db.read_only:
		# Throw away any table that was already present
		db.drop_table(table_name)
		# Create a table with the structure found in table_dict and add it to the database
		db.create_table(table_name, table_dict)

		var row_array : Array = []
		var row_dict : Dictionary = Dictionary()
		for i in range(0,ids.size()):
			row_dict["id"] = ids[i]
			row_dict["name"] = names[i]
			row_dict["age"] = ages[i]
			row_dict["address"] = addresses[i]
			row_dict["salary"] = salaries[i]
			row_array.append(row_dict.duplicate())

			# Insert a new row in the table
			db.insert_row(table_name, row_dict)
			row_dict.clear()

	# Select all employees
	var select_condition := ""
	var selected_array : Array = db.select_rows(table_name, select_condition, ["*"])
	cprint("condition: " + select_condition)
	cprint("result: {0}".format([String(selected_array)]))
	# Check the types of the values in the dictionary
	for row in selected_array:
		cprint("Types of selected columns:")
		cprint("salary: {0}".format([typeof(row["salary"])]))
		cprint("age:    {0}".format([typeof(row["age"])]))
		cprint("name:   {0}".format([typeof(row["name"])]))
		break

	# Close the current database
	db.close_db()
