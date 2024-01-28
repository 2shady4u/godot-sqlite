extends SceneTree

var data_path = "res://data"
var copy_path = "res://build/data"

func _init():
	var dir = DirAccess.open(data_path)
	if DirAccess.get_open_error() == OK:
		dir.make_dir_recursive(copy_path)
		dir.list_dir_begin();
		var file_name = dir.get_next()
		while (file_name != ""):
			if dir.current_is_dir():
				pass
			else:
				print("Copying " + file_name + " to /build-folder")
				dir.copy(data_path + "/" + file_name, copy_path + "/" + file_name)
			file_name = dir.get_next()
	else:
		print("An error occurred when trying to access the path.")
	quit()
