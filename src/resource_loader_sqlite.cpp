#include "resource_loader_sqlite.h"
#include "resource_sqlite.h"

Variant ResourceFormatLoaderSqlite::_load(const String &p_path, const String &original_path, bool use_sub_threads, int32_t cache_mode) const {
	Ref<SqliteResource> sqlite_model = memnew(SqliteResource);
	sqlite_model->set_file(p_path);
	return sqlite_model;
}
PackedStringArray ResourceFormatLoaderSqlite::_get_recognized_extensions() const {
	PackedStringArray array;
	array.push_back("bin");
	return array;
}
bool ResourceFormatLoaderSqlite::_handles_type(const StringName &type) const {
	return ClassDB::is_parent_class(type, "SqliteResource");
}
String ResourceFormatLoaderSqlite::_get_resource_type(const String &p_path) const {
	String el = p_path.get_extension().to_lower();
	if (el == "sqlite") {
		return "SqliteResource";
	}
	return "";
}
