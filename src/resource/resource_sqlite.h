#ifndef SQLITE_RESOURCE_H
#define SQLITE_RESOURCE_H

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
#endif // ! SQLITE_RESOURCE_H
