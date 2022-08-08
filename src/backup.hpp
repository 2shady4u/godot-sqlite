#ifndef HEADER_HPP
#define HEADER_HPP
#include "gdsqlite.h"

int sqlite_backup_db(
	sqlite3 *pDb,		   /* Database to back up */
	const char *zFilename, /* Name of file to back up to */
	void (*xProgress)(int, int) /* Progress function to invoke */);

int sqlite_load_to_memory(sqlite3 *pInMemory, const char *zFilename);

#endif