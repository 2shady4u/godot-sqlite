#include "gdsqlite_vfs.h"

using namespace godot;

/*
** Open a file handle.
*/
static int gdsqlite_vfs_open(sqlite3_vfs *pVfs, const char *zName, sqlite3_file *pFile, int flags, int *pOutFlags)
{
	static const sqlite3_io_methods gdsqlite_file_io_methods = {
		1,									  /* iVersion */
		gdsqlite_file::close,				  /* xClose */
		gdsqlite_file::read,				  /* xRead */
		gdsqlite_file::write,				  /* xWrite */
		gdsqlite_file::truncate,			  /* xTruncate */
		gdsqlite_file::sync,				  /* xSync */
		gdsqlite_file::fileSize,			  /* xFileSize */
		gdsqlite_file::lock,				  /* xLock */
		gdsqlite_file::unlock,				  /* xUnlock */
		gdsqlite_file::checkReservedLock,	  /* xCheckReservedLock */
		gdsqlite_file::fileControl,			  /* xFileControl */
		gdsqlite_file::sectorSize,			  /* xSectorSize */
		gdsqlite_file::deviceCharacteristics, /* xDeviceCharacteristics */
	};
	gdsqlite_file *p = reinterpret_cast<gdsqlite_file *>(pFile);
	Ref<File> file = File::_new();
	int godot_flags = 0;

	ERR_FAIL_COND_V(zName == NULL, SQLITE_IOERR); /* How does this respond to :memory:? */

	/* TODO: Add/Support additional flags: 
    ** - SQLITE_OPEN_DELETEONCLOSE
    ** - SQLITE_OPEN_EXCLUSIVE
	** - ???
	*/

	/* Convert SQLite's flags to something Godot might understand! */
	if (flags & SQLITE_OPEN_READONLY)
	{
		//Godot::print("READ");
		godot_flags |= File::READ;
	}
	if (flags & SQLITE_OPEN_READWRITE)
	{
		if (flags & SQLITE_OPEN_CREATE)
		{
			if (file->file_exists(String(zName)))
			{
				//Godot::print("READ WRITE");
				godot_flags |= File::READ_WRITE;
			}
			else
			{
				//Godot::print("WRITE READ");
				godot_flags |= File::WRITE_READ;
			}
		}
		else
		{
			//Godot::print("READ WRITE");
			godot_flags |= File::READ_WRITE;
		}
	}

	/* Attempt to open the database or journal file using Godot's `open()`-function */
	Error err_code = file->open(String(zName), godot_flags);
	if (err_code != Error::OK) {
		/* File can't be opened! */
		/* In most cases this is caused by the fact that Godot opens files in a non-shareable way, as discussed here: */
		/* https://docs.microsoft.com/en-us/cpp/c-runtime-library/reference/fopen-s-wfopen-s?view=msvc-160 */
		/* Simply assuring that the file is closed in all other programs fixes this issue... */
		/* Multiple database connections are only possible if they are opened in SQLITE_OPEN_READONLY mode */
        ERR_PRINT("GDSQLITE_VFS Error: Could not open database! Is the file read/write locked by another program? (Error = " + String(std::to_string(static_cast<int>(err_code)).c_str()) + ")");
        return SQLITE_CANTOPEN;
	}

	if (pOutFlags)
	{
		*pOutFlags = flags;
	}
	p->file = file;
	p->base.pMethods = &gdsqlite_file_io_methods;
	return SQLITE_OK;
}

/*
** Delete the file identified by argument zPath. If the dirSync parameter
** is non-zero, then ensure the file-system modification to delete the
** file has been synced to disk before returning.
*/
static int gdsqlite_vfs_delete(sqlite3_vfs *pVfs, const char *zPath, int dirSync)
{
	Ref<Directory> dir = Directory::_new();
	Error err_code = dir->remove(zPath);
	/* Probably we'll also need to check if the file exists and check the err_code! */
	return SQLITE_OK;
}

/*
** Query the file-system to see if the named file exists, is readable or
** is both readable and writable.
*/
static int gdsqlite_vfs_access(sqlite3_vfs *pVfs, const char *zPath, int flags, int *pResOut)
{
	Ref<File> file = File::_new();
	Error err_code = Error::OK;

	switch (flags)
	{
	case SQLITE_ACCESS_EXISTS:
		*pResOut = file->file_exists(zPath);
		break;

	case SQLITE_ACCESS_READWRITE:
		err_code = file->open(zPath, File::READ_WRITE);
		*pResOut = (err_code == Error::OK);
		break;

	case SQLITE_ACCESS_READ:
		err_code = file->open(zPath, File::READ);
		*pResOut = (err_code == Error::OK);
		break;

	default:
		/* Probably throw some kind of error here? */
		break;
	}

	return SQLITE_OK;
}

/*
** Argument zPath points to a nul-terminated string containing a file path.
** If zPath is an absolute path, then it is copied as is into the output 
** buffer. Otherwise, if it is a relative path, then the equivalent full
** path is written to the output buffer.
**
** This function assumes that paths are UNIX style. Specifically, that:
**
**   1. Path components are separated by a '/'. and 
**   2. Full paths begin with a '/' character.
*/
static int gdsqlite_vfs_fullPathname(sqlite3_vfs *pVfs, const char *zPath, int nPathOut, char *zPathOut)
{
	for (int i = 0; i < nPathOut; ++i)
	{
		zPathOut[i] = zPath[i];
		if (zPath[i] == '\0')
		{
			break;
		}
	}

	return SQLITE_OK;
}

/*
** The following four VFS methods:
**
**   xDlOpen
**   xDlError
**   xDlSym
**   xDlClose
**
** are supposed to implement the functionality needed by SQLite to load
** extensions compiled as shared objects. This simple VFS does not support
** this functionality, so the following functions are no-ops.
*/
static void *gdsqlite_vfs_dlOpen(sqlite3_vfs *vfs, const char *filename)
{
	return 0;
}
static void gdsqlite_vfs_dlError(sqlite3_vfs *vfs, int nBytes, char *errMsg)
{
	sqlite3_snprintf(nBytes, errMsg, "Loadable extensions are not supported");
	errMsg[nBytes - 1] = '\0';
}
static void (*gdsqlite_vfs_dlSym(sqlite3_vfs *vfs, void *data, const char *symbol))(void)
{
	return 0;
}
static void gdsqlite_vfs_dlClose(sqlite3_vfs *vfs, void *data)
{
	return;
}

/*
** Parameter zByte points to a buffer nByte bytes in size. Populate this
** buffer with pseudo-random data.
*/
static int gdsqlite_vfs_randomness(sqlite3_vfs *pVfs, int nByte, char *zByte)
{
	srand(OS::get_singleton()->get_unix_time());
	for (int i = 0; i < nByte; ++i)
	{
		zByte[i] = rand();
	}
	return SQLITE_OK;
}

/*
** Sleep for at least nMicro microseconds. Return the (approximate) number 
** of microseconds slept for.
*/
static int gdsqlite_vfs_sleep(sqlite3_vfs *pVfs, int nMicro)
{
	OS::get_singleton()->delay_usec(nMicro);
	return nMicro;
}

/*
** Set *pTime to the current UTC time expressed as a Julian day. Return
** SQLITE_OK if successful, or an error code otherwise.
**
**   http://en.wikipedia.org/wiki/Julian_day
**
** This implementation is not very good. The current time is rounded to
** an integer number of seconds. Also, assuming time_t is a signed 32-bit 
** value, it will stop working some time in the year 2038 AD (the so-called
** "year 2038" problem that afflicts systems that store time this way). 
*/
static int gdsqlite_vfs_currentTime(sqlite3_vfs *vfs, double *pTime)
{
	uint64_t unix_time = OS::get_singleton()->get_unix_time();
	*pTime = unix_time / 86400.0 + 2440587.5;
	return SQLITE_OK;
}

static int gdsqlite_vfs_getLastError(sqlite3_vfs *vfs, int nBuf, char *buf)
{
	// TODO: Implement properly
	return 0;
}

static int gdsqlite_vfs_currentTimeInt64(sqlite3_vfs *vfs, sqlite3_int64 *now)
{
	uint64_t unix_time = OS::get_singleton()->get_unix_time();
	*now = unix_time + 210866760000; // Add the number of ms since julian time
	return SQLITE_OK;
}

/*
** This function returns a pointer to the VFS implemented in this file.
** To make the VFS available to SQLite:
**
**   sqlite3_vfs_register(sqlite3_demovfs(), 0);
*/
sqlite3_vfs *godot::gdsqlite_vfs()
{
	static sqlite3_vfs godot_vfs = {
		3,							   /* iVersion */
		sizeof(gdsqlite_file),		   /* szOsFile */
		MAXPATHNAME,				   /* mxPathname */
		0,							   /* pNext */
		"godot",					   /* zName */
		NULL,						   /* pAppData */
		gdsqlite_vfs_open,			   /* xOpen */
		gdsqlite_vfs_delete,		   /* xDelete */
		gdsqlite_vfs_access,		   /* xAccess */
		gdsqlite_vfs_fullPathname,	   /* xFullPathname */
		gdsqlite_vfs_dlOpen,		   /* xDlOpen */
		gdsqlite_vfs_dlError,		   /* xDlError */
		gdsqlite_vfs_dlSym,			   /* xDlSym */
		gdsqlite_vfs_dlClose,		   /* xDlClose */
		gdsqlite_vfs_randomness,	   /* xRandomness */
		gdsqlite_vfs_sleep,			   /* xSleep */
		gdsqlite_vfs_currentTime,	   /* xCurrentTime */
		gdsqlite_vfs_getLastError,	   /* xGetLastError */
		gdsqlite_vfs_currentTimeInt64, /* xCurrentTimeInt64 */
		NULL,						   /* xSetSystemCall */
		NULL,						   /* xGetSystemCall */
		NULL						   /* xNextSystemCall */
	};
	return &godot_vfs;
}
