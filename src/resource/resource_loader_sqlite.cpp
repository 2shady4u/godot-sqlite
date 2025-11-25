#include "resource_loader_sqlite.hpp"

using namespace godot;

Variant ResourceFormatLoaderSQLite::_load(const String &p_path, const String &original_path, bool use_sub_threads, int32_t cache_mode) const {
	Ref<SQLiteResource> sqlite_model = memnew(SQLiteResource);
	sqlite_model->set_file(p_path);
	return sqlite_model;
}

PackedStringArray ResourceFormatLoaderSQLite::_get_recognized_extensions() const {
	PackedStringArray extensions = PackedStringArray();
	extensions.append(ProjectSettings::get_singleton()->get_setting(DEFAULT_EXTENSION_SETTING.c_str()));
	return extensions;
}

bool ResourceFormatLoaderSQLite::_handles_type(const StringName &type) const {
	return ClassDB::is_parent_class(type, "SQLiteResource");
}

String ResourceFormatLoaderSQLite::_get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	String default_extension = ProjectSettings::get_singleton()->get_setting(DEFAULT_EXTENSION_SETTING.c_str());
	if (el == default_extension) {
		return "SQLiteResource";
	}
	return "";
}
