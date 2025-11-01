#include "sqlite_connection_params.hpp"

using namespace godot;

void SQLiteConnectionParams::_bind_methods() {
    // Properties.
    ClassDB::bind_method(D_METHOD("set_path", "path"), &SQLiteConnectionParams::set_path);
    ClassDB::bind_method(D_METHOD("get_path"), &SQLiteConnectionParams::get_path);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "path"), "set_path", "get_path");

    ClassDB::bind_method(D_METHOD("set_read_only", "read_only"), &SQLiteConnectionParams::set_read_only);
    ClassDB::bind_method(D_METHOD("get_read_only"), &SQLiteConnectionParams::get_read_only);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "read_only"), "set_read_only", "get_read_only");

    ClassDB::bind_method(D_METHOD("set_verbosity_level", "verbosity_level"), &SQLiteConnectionParams::set_verbosity_level);
    ClassDB::bind_method(D_METHOD("get_verbosity_level"), &SQLiteConnectionParams::get_verbosity_level);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "verbosity_level"), "set_verbosity_level", "get_verbosity_level");
}

void SQLiteConnectionParams::set_path(const String &p_path) { 
    path = p_path; 
}

String SQLiteConnectionParams::get_path() const { 
    return path; 
}

void SQLiteConnectionParams::set_read_only(bool p_read_only) { 
    read_only = p_read_only; 
}

bool SQLiteConnectionParams::get_read_only() const { 
    return read_only; 
}

void SQLiteConnectionParams::set_verbosity_level(SQLiteEnums::VerbosityLevel p_verbosity_level) {
	verbosity_level = p_verbosity_level;
}

SQLiteEnums::VerbosityLevel SQLiteConnectionParams::get_verbosity_level() const {
	return verbosity_level;
}