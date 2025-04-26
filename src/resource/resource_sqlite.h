#ifndef RESOURCE_SQLITE_H
#define RESOURCE_SQLITE_H

#include <godot_cpp/classes/resource.hpp>

using namespace godot;

class SQLiteResource : public Resource {
	GDCLASS(SQLiteResource, Resource);

protected:
	static void _bind_methods() {}
	String file;

public:
	void set_file(const String &p_file);
	String get_file();
};
#endif // RESOURCE_SQLITE_H
