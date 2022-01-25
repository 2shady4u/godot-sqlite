#include "gdsqlite.h"

#include <godot_cpp/core/class_db.hpp>

#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void SQLite::_bind_methods()
{
}

SQLite::SQLite()
{
    UtilityFunctions::print("constructor");
}

SQLite::~SQLite()
{
    UtilityFunctions::print("destructor");
}