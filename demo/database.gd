extends Node

var db : SQLite = null

const verbosity_level : int = SQLite.VERBOSE

var db_name := "res://data/test"
var packaged_db_name := "res://data_to_be_packaged"
var peristent_db_name := "user://my_database"
var json_name := "res://data/test_backup"

var table_name := "company"
var other_table_name := "expenses"
var packaged_table_name = "creatures"

var ids := [1,2,3,4,5,6,7]
var names := ["Paul","Allen","Teddy","Mark","Robert","Julia","Amanda"]
var ages := [32,25,23,25,30,63,13]
var addresses := ["California","Texas","Baltimore","Richmond","Texas","Atlanta","New-York"]
var salaries := [20000.00,15000.00,20000.00,65000.00,65000.00,65000.00,65000.00]

var percentage_above_thirty := 0.05
var percentage_below_thirty := 0.1
var doomed_city := "Texas"

signal output_received(text)
signal texture_received(texture)

func _ready():
	if OS.get_name() in ["Android", "iOS", "Web"]:
		copy_data_to_user()
		db_name = "user://data/test"
		json_name = "user://data/test_backup"

	# Enable/disable examples here:
	example_of_basic_database_querying()
	example_of_in_memory_and_foreign_key_support()
	example_of_call_external_functions()
	example_of_blob_io()
	example_of_read_only_database()
	example_of_database_persistency()

func cprint(text : String) -> void:
	print(text)
	emit_signal("output_received", text)

func copy_data_to_user() -> void:
	var data_path := "res://data"
	var copy_path := "user://data"

	DirAccess.make_dir_absolute(copy_path)
	var dir = DirAccess.open(data_path)
	if dir:
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
	db.verbosity_level = verbosity_level
	# Open the database using the db_name found in the path variable
	db.open_db()
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
	#cprint(row_array)

	# Select the id and age of the employees that are older than 30
	var select_condition : String = "age > 30"
	var selected_array : Array = db.select_rows(table_name, select_condition, ["id", "age"])
	cprint("condition: " + select_condition)
	cprint("result: {0}".format([str(selected_array)]))

	# Change name of 'Amanda' to 'Olga' and her age to 30
	db.update_rows(table_name, "name = 'Amanda'", {"AGE":30, "NAME":"Olga"})

	# Select the employee with the name Olga and with age 30
	select_condition = "name = 'Olga' and age = 30"
	selected_array = db.select_rows(table_name, select_condition, ["*"])
	cprint("condition: " + select_condition)
	cprint("result: {0}".format([str(selected_array)]))

	# Delete the employee named Olga
	db.delete_rows(table_name, "name = 'Olga'")

	# Select all employees
	select_condition = ""
	selected_array = db.select_rows(table_name, select_condition, ["*"])
	cprint("condition: " + select_condition)
	cprint("result: {0}".format([str(selected_array)]))
	# Check the types of the values in the dictionary
	cprint("Types of selected columns:")
	cprint("salary: {0}".format([typeof(selected_array[0]["salary"])]))
	cprint("age:    {0}".format([typeof(selected_array[0]["age"])]))
	cprint("name:   {0}".format([typeof(selected_array[0]["name"])]))

	# Delete all employees
	db.delete_rows(table_name, "*")

	# Add all employees again
	db.insert_rows(table_name, row_array)

	# Do a normal query
	db.query("SELECT COUNT(*) AS 'number_of_employees' FROM " + table_name + ";")
	cprint("There are {0} employees in the company".format([db.query_result[0]["number_of_employees"]]))

	db.query("PRAGMA encoding;")
	cprint("Current database encoding is: {0}".format([db.query_result[0]["encoding"]]))

	# Export the table to a json-file with a specified name
	db.export_to_json(json_name + "_new")

	# Close the current database
	db.close_db()
	
	# Import (and, consequently, open) a database from an old backup json-file
	cprint("Overwriting database content with old backup...")
	db.import_from_json(json_name + "_old")

	# Check which employees were present in this old json-file
	select_condition = ""
	selected_array = db.select_rows(table_name, select_condition, ["*"])
	cprint("condition: " + select_condition)
	cprint("result: {0}".format([str(selected_array)]))
	# Check the types of the values in the dictionary
	cprint("Types of selected columns:")
	cprint("salary: {0}".format([typeof(selected_array[0]["salary"])]))
	cprint("age:    {0}".format([typeof(selected_array[0]["age"])]))
	cprint("name:   {0}".format([typeof(selected_array[0]["name"])]))

	# Import the data (in a destructive manner) from the new backup json-file
	cprint("Overwriting database content again with latest backup...")
	db.import_from_json(json_name + "_new")

	# Try to delete a non-existant table from the database.
	if not db.delete_rows(other_table_name, "*"):
		cprint("SQL error: " + db.error_message)

	# Close the imported database
	db.close_db()

# This example demonstrates the in-memory and foreign key support. It's
# rather contrived, but it gets the point across.
func example_of_in_memory_and_foreign_key_support():

	# Create the database as usual.
	db = SQLite.new()
	# Enable in-memory storage.
	db.path = ":memory:"
	db.verbosity_level = verbosity_level
	# Enable foreign keys.
	db.foreign_keys = true
	# Open the database as usual.
	db.open_db()

	# Create a table for all your friends.
	db.create_table("friends", {
		"id": {"data_type": "int", "primary_key": true, "not_null": true},
		"name": {"data_type": "text", "not_null": true, "unique": true},
		"hobby": {"data_type": "int", "foreign_key": "hobbies.id", "not_null": true}
	})

	# Create a table for all your friends' hobbies.
	db.create_table("hobbies", {
		"id": {"data_type": "int", "primary_key": true, "not_null": true},
		"description": {"data_type": "text", "not_null": true, "unique": true}
	})

	# ATTENTION: The important thing to note about the "friends" table is the
	# definition of the foreign key "hobbies.id". This tells SQLITE to enforce
	# the foreign key constraint, and that the field "friends.hobby" is now
	# tied to the field "hobbies.id". Consequently, you are now required to
	# specify a valid hobby when adding a friend to the database, which in
	# turn means you first need to add some hobbies to the database before
	# you can add any of your friends and assign them a hobby.

	# This won't work! There is no valid hobby with id 23 yet!
	db.insert_rows("friends", [
		{"id": 1, "name": "John", "hobby": 23}
	])

	# This will work! You create the hobby with id 23 first, then you can
	# create your friend referencing that hobby.
	db.insert_rows("hobbies", [
		{"id": 23, "description": "Extreme Relaxing"}
	])
	db.insert_rows("friends", [
		{"id": 1, "name": "John", "hobby": 23}
	])

	# Close the database.
	db.close_db()

func should_employee_be_fired(address : String) -> bool:
	if address == doomed_city:
		return true
	else:
		return false

func increase_wages(salary : float, age : int) -> float:
	if age > 30:
		return (1.0 + percentage_above_thirty)*salary
	else:
		return (1.0 + percentage_below_thirty)*salary

func example_of_call_external_functions():
	# Make a big table containing the variable types.
	var table_dict : Dictionary = Dictionary()
	table_dict["id"] = {"data_type":"int", "primary_key": true, "not_null": true}
	table_dict["name"] = {"data_type":"text", "not_null": true}
	table_dict["age"] = {"data_type":"int", "not_null": true}
	table_dict["address"] = {"data_type":"char(50)"}
	table_dict["salary"] = {"data_type":"real"}

	db = SQLite.new()
	db.path = db_name
	db.verbosity_level = verbosity_level
	# Open the database using the db_name found in the path variable
	db.open_db()
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

	var callable := Callable(self, "should_employee_be_fired")
	db.create_function("should_employee_be_fired", callable, 1)

	callable = Callable(self, "increase_wages")
	db.create_function("increase_wages", callable, 2)

	db.query("UPDATE company SET salary = increase_wages(salary, age);")
	db.query("DELETE FROM company WHERE should_employee_be_fired(address);")

	var select_condition := ""
	var selected_array : Array = db.select_rows(table_name, select_condition, ["id", "salary", "name"])
	cprint("result: {0}".format([str(selected_array)]))

# The BLOB-datatype is useful when lots of raw data has to be stored.
# For example images fall into this category!
func example_of_blob_io():
	# Make a big table containing the variable types.
	var table_dict : Dictionary = Dictionary()
	table_dict["id"] = {"data_type":"int", "primary_key": true, "not_null": true}
	table_dict["data"] = {"data_type":"blob", "not_null": true}

	var texture := preload("res://icon.png")
	emit_signal("texture_received", texture)
	var tex_data : PackedByteArray = texture.get_image().save_png_to_buffer()

	db = SQLite.new()
	db.path = db_name
	db.verbosity_level = verbosity_level
	# Open the database using the db_name found in the path variable
	db.open_db()
	# Throw away any table that was already present
	db.drop_table(table_name)
	# Create a table with the structure found in table_dict and add it to the database
	db.create_table(table_name, table_dict)

	# Insert a new row in the table and bind the texture data to the data column.
	db.insert_row(table_name, {"id": 1, "data": tex_data})

	var selected_array : Array = db.select_rows(table_name, "", ["data"])
	for selected_row in selected_array:
		var selected_data = selected_row.get("data", PackedByteArray())

		var image := Image.new()
		var _error : int = image.load_png_from_buffer(selected_data)
		var loaded_texture := ImageTexture.create_from_image(image)
		emit_signal("texture_received", loaded_texture)

	# Export the table to a json-file and automatically encode BLOB data to base64.
	db.export_to_json(json_name + "_base64_new")

	# Import again!
	db.import_from_json(json_name + "_base64_old")

	# Check out the 'old' icon stored in this backup file!
	selected_array = db.select_rows(table_name, "", ["data"])
	for selected_row in selected_array:
		var selected_data = selected_row.get("data", PackedByteArray())

		var image := Image.new()
		var _error : int = image.load_png_from_buffer(selected_data)
		var loaded_texture := ImageTexture.create_from_image(image)
		emit_signal("texture_received", loaded_texture)

	# Close the current database
	db.close_db()

func regexp(pattern : String, subject : String) -> bool:
	var regex = RegEx.new()
	regex.compile(pattern)
	var result = regex.search(subject)
	if result:
		return true
	else:
		return false

# Example of accessing a packaged database by using the custom Virtual File System (VFS)
# which allows packaged databases to be opened in read_only modus.
# Databases used in this way can be added to the project's export filters
# and will be readable, but not writable, by Godot.
func example_of_read_only_database():
	db = SQLite.new()
	db.path = packaged_db_name
	db.verbosity_level = verbosity_level
	db.read_only = true

	db.open_db()

	var callable := Callable(self, "regexp")
	db.create_function("regexp", callable, 2)

	# Select all the creatures
	var select_condition : String = ""
	var selected_array : Array = db.select_rows(packaged_table_name, select_condition, ["*"])
	cprint("condition: " + select_condition)
	cprint("result: {0}".format([str(selected_array)]))

	# Select all the creatures that start with the letter 'b'
	select_condition = "name LIKE 'b%'"
	selected_array = db.select_rows(packaged_table_name, select_condition, ["name"])
	cprint("condition: " + select_condition)
	cprint("Following creatures start with the letter 'b':")
	for row in selected_array:
		cprint("* " + row["name"])

	# Do the same thing by using the REGEXP operator
	# This function has to be user-defined as discussed here:
	# https://www.sqlite.org/lang_expr.html#regexp
	select_condition = "name REGEXP '^s.*'"
	selected_array = db.select_rows(packaged_table_name, select_condition, ["name"])
	cprint("condition: " + select_condition)
	cprint("Following creatures start with the letter 's':")
	for row in selected_array:
		cprint("* " + row["name"])

	# Open another simultanous database connection in read-only mode.
	# Having multiple database connections in read-only mode is allowed.
	# Having multiple database connections in read and write is NOT allowed!
	# This behaviour is hard-coded into Godot's file handling system and can't be
	# modified by this plugin.
	var other_db = SQLite.new()
	other_db.path = packaged_db_name
	other_db.verbosity_level = verbosity_level
	other_db.read_only = true

	other_db.open_db()

	# Get the experience you would get by kiling a mimic.
	select_condition = "name = 'mimic'"
	selected_array = other_db.select_rows(packaged_table_name, select_condition, ["experience"])
	cprint("Killing a mimic yields " + str(selected_array[0]["experience"]) + " experience points!")

	# Close the current database
	db.close_db()

func example_of_database_persistency():
	var table_dict : Dictionary = Dictionary()
	table_dict["id"] = {"data_type":"int", "primary_key": true, "not_null": true}
	table_dict["count"] = {"data_type":"int", "not_null": true, "default": 0}

	db = SQLite.new()
	db.path = peristent_db_name
	db.verbosity_level = verbosity_level
	db.open_db()
	db.create_table(table_name, table_dict)

	# Does the row already exist?
	db.select_rows(table_name, "id = 1", ["count"])
	var query_result : Array = db.query_result
	var count : int = 0
	if query_result.is_empty():
		# It doesn't exist yet! Add it!
		db.insert_row(table_name, {"id": 1, "count": 0})
	else:
		var result : Dictionary = query_result[0]
		count = int(result.get("count", count))

	cprint("Count is: {0}".format([count]))

	# Increment the value for the next time!
	db.update_rows(table_name, "id = 1", {"count": count + 1 })

	# Close the current database
	db.close_db()
