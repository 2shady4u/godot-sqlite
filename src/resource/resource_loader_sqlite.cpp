#include "resource_loader_sqlite.h"
#include "resource_sqlite.h"
#include <godot_cpp/classes/project_settings.hpp>

using namespace godot;

Variant ResourceFormatLoaderSQLite::_load(const String &p_path, const String &original_path, bool use_sub_threads, int32_t cache_mode) const {
	Ref<SQLiteResource> sqlite_model = memnew(SQLiteResource);
	sqlite_model->set_file(p_path);
	return sqlite_model;
}

PackedStringArray ResourceFormatLoaderSQLite::_get_recognized_extensions() const {
	PackedStringArray extensions = PackedStringArray();
	extensions.append(ProjectSettings::get_singleton()->get("filesystem/import/sqlite/default_extension"));
	return extensions;
}

bool ResourceFormatLoaderSQLite::_handles_type(const StringName &type) const {
	return ClassDB::is_parent_class(type, "SQLiteResource");
}

String ResourceFormatLoaderSQLite::_get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	for (String element : (PackedStringArray)ProjectSettings::get_singleton()->get("filesystem/import/sqlite/default_extension")) {
		if (el == element) {
			return "SQLiteResource";
		}
	}
	return "";
}
