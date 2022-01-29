extends Node

var db : SQLite = null

var db_name := "res://data/test"

var table_name := "company"

var ids := [1,2,3,4,5,6,7]
var names := ["Paul","Allen","Teddy","Mark","Robert","Julia","Amanda"]
var ages := [32,25,23,25,30,63,13]
var addresses := ["California","Texas","Baltimore","Richmond","Texas","Atlanta","New-York"]
var salaries := [20000.00,15000.00,20000.00,65000.00,65000.00,65000.00,65000.00]

signal output_received(text)
signal texture_received(texture)

func _ready():
	# Enable/disable examples here:
	example_of_basic_database_querying()

func cprint(text : String) -> void:
	print(text)
	emit_signal("output_received", text)

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
	cprint("result: {0}".format([selected_array]))

	# Change name of 'Amanda' to 'Olga' and her age to 30
	db.update_rows(table_name, "name = 'Amanda'", {"AGE":30, "NAME":"Olga"})

	# Select the employee with the name Olga and with age 30
	select_condition = "name = 'Olga' and age = 30"
	selected_array = db.select_rows(table_name, select_condition, ["*"])
	cprint("condition: " + select_condition)
	cprint("result: {0}".format([selected_array]))

	# Delete the employee named Olga
	db.delete_rows(table_name, "name = 'Olga'")

	# Select all employees
	select_condition = ""
	selected_array = db.select_rows(table_name, select_condition, ["*"])
	cprint("condition: " + select_condition)
	cprint("result: {0}".format([selected_array]))
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

	# Close the current database
	db.close_db()
