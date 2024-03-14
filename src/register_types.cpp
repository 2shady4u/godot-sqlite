#include "register_types.h"

#include <gdextension_interface.h>

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/classes/project_settings.hpp>

#include "gdsqlite.h"
#include "resource_loader_sqlite.h"
#include "resource_sqlite.h"

using namespace godot;

static Ref<ResourceFormatLoaderSQLite> sqlite_loader;

void register_setting(
		const String &p_name,
		const Variant &p_value,
		PropertyHint p_hint,
		const String &p_hint_string,
    bool restart_if_changed) {
	ProjectSettings *project_settings = ProjectSettings::get_singleton();

	if (!project_settings->has_setting(p_name)) {
		project_settings->set(p_name, p_value);
	}

	Dictionary property_info;
	property_info["name"] = p_name;
	property_info["type"] = p_value.get_type();
	property_info["hint"] = p_hint;
	property_info["hint_string"] = p_hint_string;

	project_settings->add_property_info(property_info);
	project_settings->set_initial_value(p_name, p_value);
  project_settings->set_restart_if_changed(p_name, true);
}

void initialize_sqlite_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	GDREGISTER_CLASS(SQLite);
	GDREGISTER_CLASS(ResourceFormatLoaderSQLite);
	GDREGISTER_CLASS(SQLiteResource);

	sqlite_loader.instantiate();
	ResourceLoader::get_singleton()->add_resource_format_loader(sqlite_loader);
  PackedStringArray array;
	array.push_back("db");
	register_setting("filesystem/import/sqlite/default_extension", array, PROPERTY_HINT_NONE, {}, true);
}

void uninitialize_sqlite_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}
	ResourceLoader::get_singleton()->remove_resource_format_loader(sqlite_loader);
	sqlite_loader.unref();
}

extern "C" {

// Initialization.

GDExtensionBool GDE_EXPORT sqlite_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
	godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

	init_obj.register_initializer(initialize_sqlite_module);
	init_obj.register_terminator(uninitialize_sqlite_module);
	init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

	return init_obj.init();
}
}
