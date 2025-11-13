#ifndef SQLITE_CONNECTION_PARAMS_CLASS_H
#define SQLITE_CONNECTION_PARAMS_CLASS_H

#include <godot_cpp/classes/ref_counted.hpp>

#include <sqlite_enums.hpp>

namespace godot {

class SQLiteConnectionParams : public RefCounted {
    GDCLASS(SQLiteConnectionParams, RefCounted);

protected:
    static void _bind_methods();

private:
    String path = "database.db";
	bool read_only = false;
    SQLiteEnums::VerbosityLevel verbosity_level = SQLiteEnums::VerbosityLevel::NORMAL;

public:
    void set_path(const String &p_path);
    String get_path() const;

	void set_read_only(bool p_read_only);
	bool get_read_only() const;

    void set_verbosity_level(SQLiteEnums::VerbosityLevel p_verbosity_level);
    SQLiteEnums::VerbosityLevel get_verbosity_level() const;
};

} //namespace godot

#endif // ! SQLITE_CONNECTION_PARAMS_CLASS_H
