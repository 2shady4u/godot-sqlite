#ifndef GDSQLITE_VFS_H
#define GDSQLITE_VFS_H

#include <godot_cpp/variant/utility_functions.hpp>

#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/time.hpp>

#include "./sqlite/sqlite3.h"
#include "gdsqlite_file.h"
#include <iostream>
#include <sstream>

/*
** The maximum pathname length supported by this VFS.
*/
#define MAXPATHNAME 512

namespace godot {

sqlite3_vfs *gdsqlite_vfs();

}

#endif