#include "gdsqlite.h"

// source from https://www.sqlite.org/backup.html

int sqlite_backup_db(
	sqlite3 *pDb,
	const char *zFilename,
	void (*xProgress)(int, int))
{
	int rc;
	sqlite3 *pFile;
	sqlite3_backup *pBackup;

	rc = sqlite3_open(zFilename, &pFile);
	if (rc == SQLITE_OK)
	{

		pBackup = sqlite3_backup_init(pFile, "main", pDb, "main");
		if (pBackup)
		{

			do
			{
				rc = sqlite3_backup_step(pBackup, 5);
				xProgress(
					sqlite3_backup_remaining(pBackup),
					sqlite3_backup_pagecount(pBackup));
				if (rc == SQLITE_OK || rc == SQLITE_BUSY || rc == SQLITE_LOCKED)
				{
					sqlite3_sleep(250);
				}
			} while (rc == SQLITE_OK || rc == SQLITE_BUSY || rc == SQLITE_LOCKED);

			(void)sqlite3_backup_finish(pBackup);
		}
		rc = sqlite3_errcode(pFile);
	}

	(void)sqlite3_close(pFile);
	return rc;
}

int sqlite_load_to_memory(sqlite3 *pInMemory, const char *zFilename)
{
	int rc;
	sqlite3 *pFile;
	sqlite3_backup *pBackup;
	sqlite3 *pTo;
	sqlite3 *pFrom;

	rc = sqlite3_open(zFilename, &pFile);
	if (rc == SQLITE_OK)
	{
		pFrom = pFile;
		pTo = pInMemory;

		pBackup = sqlite3_backup_init(pTo, "main", pFrom, "main");
		if (pBackup)
		{
			(void)sqlite3_backup_step(pBackup, -1);
			(void)sqlite3_backup_finish(pBackup);
		}
		rc = sqlite3_errcode(pTo);
	}

	(void)sqlite3_close(pFile);
	return rc;
}