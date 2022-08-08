extends Node2D

const SQLite = preload("res://addons/godot-sqlite/bin/gdsqlite.gdns")
var db
var path = "data/jip.db"

var d = {}


func read_sql_file(filepath: String):
	var file := File.new()
	file.open(filepath, file.READ)
	var content = file.get_as_text()
	file.close()
	return content


func saveFile():
	var configFile = ConfigFile.new()
	configFile.set_value("main", "name", "Jacson")
	configFile.save("data/config")


# Called when the node enters the scene tree for the first time.
func _init():
	db = SQLite.new()
	db.path = ":memory:"
	db.open_db()

	db.recovery(path)

	var init_sql = read_sql_file("init.sql")
	db.exec(init_sql)

	var table_name = "apple"

	var dict: Dictionary = Dictionary()
	dict["name"] = "app mk2"
	dict["weight"] = 32

	db.insert_row(table_name, dict)

	var now = Time.get_ticks_usec()
	var last = 0
	print(" ............. ", db.error_message)

	db.backup(path)

	last = now
	now = Time.get_ticks_usec()
	print(now - last, " ............. ", db.error_message)

	db.backup(path)

	last = now
	now = Time.get_ticks_usec()
	print(now - last, " ............. ")

	saveFile()

	last = now
	now = Time.get_ticks_usec()
	print(now - last, " ............. ")

	pass  # Replace with function body.

# Called every frame. 'delta' is the elapsed time since the previous frame.
#func _process(delta):
#	pass
