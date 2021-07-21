#ifndef GDSQLITE_VFS_H
#define GDSQLITE_VFS_H

#include <Directory.hpp>

#include "./sqlite/sqlite3.h"
#include "gdsqlite_file_vfs.h"

/*
** The maximum pathname length supported by this VFS.
*/
#define MAXPATHNAME 512

namespace godot
{

    sqlite3_vfs* gdsqlite_vfs();

}

#endif