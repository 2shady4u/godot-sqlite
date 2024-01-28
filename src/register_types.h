#ifndef SQLITE_REGISTER_TYPES_H
#define SQLITE_REGISTER_TYPES_H

#include <godot_cpp/core/class_db.hpp>
using namespace godot;

void initialize_sqlite_module(ModuleInitializationLevel p_level);
void uninitialize_sqlite_module(ModuleInitializationLevel p_level);

#endif // ! SQLITE_REGISTER_TYPES_H