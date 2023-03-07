extends GutTest

var db_name := "res://data/test"
var db

#func copy_data_to_user() -> void:
#	var data_path := "res://data"
#	var copy_path := "user://data"
#
#	var dir = Directory.new()
#	dir.make_dir(copy_path)
#	if dir.open(data_path) == OK:
#		dir.list_dir_begin();
#		var file_name = dir.get_next()
#		while (file_name != ""):
#			if dir.current_is_dir():
#				pass
#			else:
#				gut.p("Copying " + file_name + " to /user-folder")
#				dir.copy(data_path + "/" + file_name, copy_path + "/" + file_name)
#			file_name = dir.get_next()
#	else:
#		gut.p("An error occurred when trying to access the path.")

#func before_all():
#	if OS.get_name() in ["Android", "iOS", "HTML5"]:
#		copy_data_to_user()
#		db_name = "user://data/test"
#	else:
#		gut.p(OS.get_name())

func test_open_close():
	db = SQLite.new()
	db.path = db_name
	assert_true(db.open_db())
	db.close_db()
