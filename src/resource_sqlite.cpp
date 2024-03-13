#include "resource_sqlite.h"
#include <iostream>

#include <godot_cpp/classes/file_access.hpp>

PackedByteArray SQLiteResource::get_content() {
	PackedByteArray content;
	String p_path = get_file();
	content = FileAccess::get_file_as_bytes(p_path);
	return content;
}
