#ifndef GDSQLITE_VFS_H
#define GDSQLITE_VFS_H

#include <Godot.hpp>
#include <Directory.hpp>

#include <sstream>
#include <iostream>
#include "./sqlite/sqlite3.h"
#include "gdsqlite_file.h"

/*
** The maximum pathname length supported by this VFS.
*/
#define MAXPATHNAME 512

namespace godot
{

    sqlite3_vfs* gdsqlite_vfs();

}

#endif