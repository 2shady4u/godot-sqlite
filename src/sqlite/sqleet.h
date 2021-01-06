/*
 * sqleet configuration.
 *
 * # SKIP_HEADER_BYTES
 * Keep this many bytes unencrypted in the beginning of the database header.
 * Value 0 produces encrypted databases that are indistinguishable from random.
 * Use 24 for compatibility with the SQLite3 Encryption Extension (SEE) so that
 * database settings (e.g., page size) can be read from an encrypted database.
 * Alternatively, 'skip' and 'page_size' URI parameters can be used at run-time
 * to specify the number of unencrypted bytes as well as the database page size.
 */
#ifndef SKIP_HEADER_BYTES
#define SKIP_HEADER_BYTES 0
#endif

/*
 * SQLite3 configuration.
 *
 * # SQLITE_HAS_CODEC
 * Required for compiling SQLite3 with encryption support.
 *
 * # SQLITE_TEMP_STORE
 * - 0 Store temp files on disk
 * - 1 Store temp files on disk but allow overriding with `PRAGMA temp_store`
 * - 2 Store temp files in memory but allow overriding with `PRAGMA temp_store`
 * - 3 Store temp files on memory
 * Note that temp files are *NOT* encrypted so using either 2 or 3 is critical!
 */
#ifndef SQLITE_HAS_CODEC
#define SQLITE_HAS_CODEC 1
#endif
#ifndef SQLITE_TEMP_STORE
#define SQLITE_TEMP_STORE 2
#endif

/* Omit "sqlite3.h" when including this header from "sqlite3.c" */
#ifndef SQLITE3_H_OMIT
#include "sqlite3.h"
#endif
