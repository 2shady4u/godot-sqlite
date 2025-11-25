#include "resource_sqlite.hpp"

using namespace godot;

void SQLiteResource::set_file(const String &p_file) {
	file = p_file;
	emit_changed();
}

String SQLiteResource::get_file() {
	return file;
}
