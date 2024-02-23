#ifndef SQLITE_RESOURCE_H
#define SQLITE_RESOURCE_H

#include <godot_cpp/classes/resource.hpp>

using namespace godot;

class SqliteResource : public Resource {
	GDCLASS(SqliteResource, Resource);

protected:
	static void _bind_methods() {}
	String file;

public:
	void set_file(const String &p_file) {
		file = p_file;
		emit_changed();
	}

	String get_file() {
		return file;
	}

	PackedByteArray get_content();
	SqliteResource() {}
	~SqliteResource() {}
};
#endif // SQLITE_RESOURCE_H
