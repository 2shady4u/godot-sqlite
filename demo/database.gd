extends Node

const SQLite = preload("res://bin/gdsqlite.gdns")
var db
#var db_name = "user://test"
var db_name = "res://test"
#var db_name = "test"
var json_name = "test_backup"
var table_name = "company"

var ids = [1,2,3,4,5,6,7]
var names = ["Paul","Allen","Teddy","Mark","Robert","Julia","Amanda"]
var ages = [32,25,23,25,30,63,13]
var addresses = ["California","Texas","Baltimore","Richmond","Texas","Atlanta","New-York"]
var salaries = [20000.00,15000.00,20000.00,65000.00,65000.00,65000.00,65000.00]

func _ready():
	
	var table_dict = Dictionary()
	table_dict["id"] = {"data_type":"int", "primary_key": true, "not_null": true}
	table_dict["name"] = {"data_type":"text", "not_null": true}
	table_dict["age"] = {"data_type":"int", "not_null": true}
	table_dict["address"] = {"data_type":"char(50)"}
	table_dict["salary"] = {"data_type":"real"}
	#print(table_dict)
	
	db = SQLite.new()
	db.path = db_name
	db.verbose_mode = true
	# Open the database using the db_name found in the path variable
	db.open_db()
	# Throw away any table that was already present
	db.drop_table(table_name)
	# Create a table with the structure found in table_dict and add it to the database
	db.create_table(table_name, table_dict)
	
	var row_array = []
	var row_dict = Dictionary()
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
	#print(row_array)

	# Select the id and age of the employees that are older than 30
	var select_condition = "age > 30"
	var selected_array = db.select_rows(table_name, select_condition, ["id", "age"])
	print("condition: " + select_condition)
	print("result: ", selected_array)
	
	# Change name of 'Amanda' to 'Olga' and her age to 30
	db.update_rows(table_name, "name = 'Amanda'", {"AGE":30, "NAME":"Olga"})

	# Select the employee with the name Olga and with age 30
	select_condition = "name = 'Olga' and age = 30"
	selected_array = db.select_rows(table_name, select_condition, ["*"])
	print("condition: " + select_condition)
	print("result: ", selected_array)

	# Delete the employee named Olga
	db.delete_rows(table_name, "name = 'Olga'")

	# Select all employees
	select_condition = ""
	selected_array = db.select_rows(table_name, select_condition, ["*"])
	print("condition: " + select_condition)
	print("result: ", selected_array)
	# Check the types of the values in the dictionary
	print("Types of selected columns:")
	print("salary: ", typeof(selected_array[0]["salary"]))
	print("age:    ", typeof(selected_array[0]["age"]))
	print("name:   ", typeof(selected_array[0]["name"]))

	# Delete all employees
	db.delete_rows(table_name, "*")

	# Add all employees again
	db.insert_rows(table_name, row_array)

	# Do a normal query
	db.query("SELECT COUNT(*) AS 'number_of_employees' FROM " + table_name + ";")
	print("There are ", db.query_result[0]["number_of_employees"], " employees in the company")
	
	db.query("PRAGMA encoding;")
	print("Current database encoding is: ", db.query_result[0]["encoding"])
	
	# Export the table to a json-file with a specified name
	db.export_to_json("data/" + json_name + "_new")
	
	# Close the current database
	db.close_db()

	# Import (and, consequently, open) a database from an old backup json-file
	print("Overwriting database content with old backup...")
	db.import_from_json("data/" + json_name + "_old")
	
	# Check which employees were present in this old json-file
	select_condition = ""
	selected_array = db.select_rows(table_name, select_condition, ["*"])
	print("condition: " + select_condition)
	print("result: ", selected_array)
	# Check the types of the values in the dictionary
	print("Types of selected columns:")
	print("salary: ", typeof(selected_array[0]["salary"]))
	print("age:    ", typeof(selected_array[0]["age"]))
	print("name:   ", typeof(selected_array[0]["name"]))
	
	# Import the data (in a destructive manner) from the new backup json-file
	print("Overwriting database content again with latest backup...")
	db.import_from_json("data/" + json_name + "_new")
	
	# Close the imported database
	db.close_db()
	
	
	
	