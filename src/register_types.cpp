#include "register_types.h"

#include <gdextension_interface.h>

#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

#include "constants.h"
#include "gdsqlite.h"
#include "resource/resource_loader_sqlite.h"
#include "resource/resource_sqlite.h"

using namespace godot;

static Ref<ResourceFormatLoaderSQLite> sqlite_loader;

void initialize_sqlite_module(ModuleInitializationLevel p_level) {
	if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
		return;
	}

	GDREGISTER_CLASS(SQLite);
	GDREGISTER_CLASS(ResourceFormatLoaderSQLite);
	GDREGISTER_CLASS(SQLiteResource);

	sqlite_loader.instantiate();
	ResourceLoader::get_singleton()->add_resource_format_loader(sqlite_loader);

	ProjectSettings *project_settings = ProjectSettings::get_singleton();
	if (!project_settings->has_setting(DEFAULT_EXTENSION_SETTING.c_str())) {
		project_settings->set_setting(DEFAULT_EXTENSION_SETTING.c_str(), DEFAULT_EXTENSION_VALUE.c_str());
	}

	Dictionary property_info;
	property_info["name"] = DEFAULT_EXTENSION_SETTING.c_str();
	property_info["type"] = godot::Variant::Type::STRING;

	project_settings->add_property_info(property_info);
	project_settings->set_initial_value(DEFAULT_EXTENSION_SETTING.c_str(), DEFAULT_EXTENSION_VALUE.c_str());
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
