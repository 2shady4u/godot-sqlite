#ifndef GDSQLITE_FILE_H
#define GDSQLITE_FILE_H

#include <Godot.hpp>
#include <File.hpp>
#include <OS.hpp>

#include "./sqlite/sqlite3.h"

namespace godot
{
    struct gdsqlite_file
    {
        sqlite3_file base; /* Base class. Must be first. */
        Ref<File> file;    /* File descriptor */

        static int close(sqlite3_file *pFile);
        static int read(sqlite3_file *pFile, void *zBuf, int iAmt, sqlite_int64 iOfst);
        static int write(sqlite3_file *pFile, const void *zBuf, int iAmt, sqlite_int64 iOfst);
        static int truncate(sqlite3_file *pFile, sqlite_int64 size);
        static int sync(sqlite3_file *pFile, int flags);
        static int fileSize(sqlite3_file *pFile, sqlite_int64 *pSize);
        static int lock(sqlite3_file *pFile, int eLock);
        static int unlock(sqlite3_file *pFile, int eLock);
        static int checkReservedLock(sqlite3_file *pFile, int *pResOut);
        static int fileControl(sqlite3_file *pFile, int op, void *pArg);
        static int sectorSize(sqlite3_file *pFile);
        static int deviceCharacteristics(sqlite3_file *pFile);
    };

}

#endif