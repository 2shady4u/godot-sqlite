#ifndef SQLITE_CONSTANTS_H
#define SQLITE_CONSTANTS_H

#include <string>

/* NOTE: We can't use StringName here as these lines run during static initialization. */
/* If using StringName -> Constructor touches uninitialized global state -> CRASHES! */

const std::string DEFAULT_EXTENSION_SETTING = "filesystem/import/sqlite/default_extension";
const std::string DEFAULT_EXTENSION_VALUE = "db";

#endif // ! SQLITE_CONSTANTS_H