#include "resource_sqlite.h"
#include <iostream>

#include <godot_cpp/classes/file_access.hpp>


void SQLiteResource::set_file(const String &p_file) {
	file = p_file;
	emit_changed();
}

String SQLiteResource::get_file() {
	return file;
}
