![Godot SQLite banner](icon/godot-sqlite-banner-v2.png?raw=true "Godot SQLite Banner")

# godot-sqlite

This GDNative script aims to serve as a custom wrapper that makes SQLite3 available in Godot 3.1+. Additionally, it
does not require any additional compilation or mucking about with build scripts.

### Supported operating systems:
- Mac OS X
- Linux
- Windows
- Android (arm64-v8a, armeabi-v7a & x86)
- iOS (arm64 & armv7)

_DISLAIMER_: iOS is still untested! (as of 24/12/2020)

# How to install?

Re-building Godot from scratch is **NOT** required, the proper way of installing this plugin is to either install it through the Asset Library or to just manually download the build files yourself.

### Godot Asset Library

**Godot-SQLite** is available through the official Godot Asset Library, and can be installed in the following way:

- Click on the 'AssetLib' button at the top of the editor.
- Search for 'godot-sqlite' and click on the resulting element.
- In the dialog pop-up, click 'Download'.
- Once the download is complete, click on the install button...
- Once more, click on the 'Install' button.
- Activate the plugin in the 'Project Settings/Plugins'-menu.
- All done!

### Manually

It's also possible to manually download the build files found in the [releases](https://github.com/2shady4u/godot-sqlite/releases) tab, extract them on your system and run the supplied demo-project. Make sure that Godot is correctly loading the *gdsqlite.gdns*-resource and that it is available in the *res://*-environment.

An example project, named "demo", can also be downloaded from the releases tab.

## Variables

- **path** (String ,default value='default')

Path to the database, should be set before opening the database with .open_db(). If no database with this name exists, a new one at the supplied path will be created. Both *res://* and *user://* keywords can be used to define the path.

- **error_message** (String, default='')

Contains the zErrMsg returned by the SQLite query in human-readable form. An empty string corresponds with the case in which the query executed succesfully.

- **verbose_mode** (Boolean, default=false)

Setting verbose_mode on True results in an information dump in the Godot console that is handy for debugging your (possibly faulty) SQLite queries.

- **foreign_keys** (Boolean, default=false)

Enables or disables the availability of [foreign keys](https://www.sqlite.org/foreignkeys.html) in the SQLite database.

- **query_result** (Array, default=[])

Contains the results from the latest query and is cleared after every new query.

## Functions

- Boolean success = **open_db()**

- Boolean success = **import_from_json(** String import_path **)**

Drops all database tables and imports the database structure and content present inside of import_path.json.

- Boolean success = **export_to_json(** String export_path **)**

Exports the database structure and content to export_path.json as a backup or for ease of editing.

- Boolean success = **close_db()**

- Boolean success = **query(** String query_string **)**

- Boolean success = **create_table(** String table_name, Dictionary table_dictionary **)**

Each key/value pair of the `table_dictionary`-variable defines a column of the table. Each key defines the name of a column in the database, while the value is a dictionary that contains further column specifications.

**Required fields**:

- **"data_type"**: type of the column variable, following values are valid\*:

    | value       | SQLite         | Godot       |
    |:-----------:|:--------------:|:-----------:|
    | int         | INTEGER        | TYPE_INT    |
    | real        | REAL           | TYPE_REAL   |
    | text        | TEXT           | TYPE_STRING |
    | char(?)\*\* | CHAR(?)\*\*    | TYPE_STRING |

    \* *Data types not found in this table are automatically converted to TYPE_STRING*  
    \*\* *with the question mark being replaced by the maximum amount of characters*

**Optional fields**:

- **"not_null"** *(default = false)*: Is the NULL value an invalid value for this column?

- **"default"**: The default value of the column if not explicitly given.

- **"primary_key"** *(default = false)*: Is this the primary key of this table?  
Evidently, only a single column can be set as the primary key.

- **"auto_increment"** *(default = false)*: Automatically increment this column when no explicit value is given. This auto-generated value will be one more (+1) than the largest value currently in use.

    ***NOTE**: Auto-incrementing a column only works when this column is the primary key!*

- **"foreign_key"**: Enforce an "exist" relationship between tables by setting this variable to `foreign_table.foreign_column`. In other words, when adding an additional row, the column value should be an existing value as found in the column with name `foreign_column` of the table with name `foreign_table`.

    ***NOTE**: Availability of foreign keys has to be enabled by setting the `foreign_keys`-variable to `True`.*

**Example usage**:

```Swift
# Add the row "id" to the table, which is an auto-incremented primary key.
# When adding additional rows, this value can either by explicitely given or be unfilled.
table_dictionary["id"] = {
    "data_type":"int", 
    "primary_key": true, 
    "auto_increment":true
}
```

For more concrete usage examples see the `database.gd`-file as found in this repository's demo project.

- Boolean success = **drop_table(** String table_name **)**

- Boolean success = **insert_row(** String table_name, Dictionary row_dictionary **)**

Each key/value pair of the `row_dictionary`-variable defines the column values of a single row.  

Columns should adhere to the table schema as instantiated using the `table_dictionary`-variable and are required if their corresponding **"not_null"**-column value is set to `True`.

- Boolean success = **insert_rows(** String table_name, Array row_array **)**

- Array selected_rows = **select_rows(** String table_name, String query_conditions, Array selected_columns **)**

- Boolean success = **update_rows(** String table_name, String query_conditions, Dictionary updated_row_dictionary **)**

With the `update_row_dictionary`-variable adhering to the same table schema & conditions as the `row_dictionary`-variable defined previously.

- Boolean success = **delete_rows(** String table_name, String query_conditions **)**

- Boolean success = **create_function(** String function_name, FuncRef function_reference, int number_of_arguments **)**

Bind a [scalar SQL function](https://www.sqlite.org/appfunc.html) to the database that can then be used in subsequent queries.

## Frequently Asked Questions (FAQ)

### 1. My query fails and returns syntax errors, what should I do?

There are a couple of things you can do before panicking, namely:
- Test out if your query is valid by trying it out online at https://sqliteonline.com/.
- Encapsulate all conditional statements in tick-marks ', for example:

The following statement might not work and throw syntax errors:
```swift
var rows = db.select_rows(table_name, "number >= {0} AND number < {0} + {1}".format([i, step]), ["*"])
```

Encapsulating the conditonal statements with tick-marks ' fixes these syntax errors:
```swift
var rows = db.select_rows(table_name, "number >= {0} AND number < {0} + {1}".format([i, step]), ["*"])
```
This issue is still open and is being actively worked on.

- Using the **query(** **)**-function instead of the more specialized wrapper function.

After exhausting these options, please open an issue that describes the error in detail.

### 2. When should I create function bindings to augment SQLite's set of native functions?

Preferably never.

Creating function should only be seen as a measure of last resort and only be used when you perfectly know what you are doing. Be sure to first check out the available native list of [scalar SQL applications](https://www.sqlite.org/lang_corefunc.html) that is already available in SQLite3.

### 2. My Android (or iOS) application cannot access the database!

Android does not allow modification of files in the 'res://'-folder, thus blocking the plugin from writing to and/or reading from this database-file.
In both cases, the most painless solution is to copy the entire database to the 'user://-folder' as apps have explicit writing privileges there.

If there is a better solution, one that does not involve copying the database to a new location, please do enlighten me.

# How to export?

All json- and db-files should be part of the exact same folder (demo/data in the case of the demo-project). During export this folder should be copied in its entirety to the demo/build-folder, in which the executable will be created by Godot's export command line utilities. Luckily, a Godot script called 'export_data.gd' can also found in the demo-project and allows to automatically copy the demo/data-folder's contents to the demo/build-folder.

The commands to succesfully export a working executable for the demo-project are:
```
mkdir build
godot -s export_data.gd
godot -e --export-debug "Windows Desktop" build/godot-sqlite.exe
```
("Windows Desktop" should be replaced by whatever the name of your relevant build template is.)

The design philosophy behind this is the following:

- Even after exporting the entire game, the data-files can still be easily edited without requiring additional exports for every miniscule edit.
- Importing a database from a json-file allows the Godot executable to use the full capabilities of the SQLite framework (SELECT, UPDATE, ...), while still allowing writers and other content creators to edit these json-files using their favorite text-based IDE.

# How to contribute?

First clone the project and install SCons. Secondly, the C++ bindings have to be build from scratch using the files present in the godot-cpp submodule and following command:

```
scons p=<platform> bits=64 generate_bindings=yes -j4
```

In the case of Android and iOS, additional parameters have to be supplied to specify the architecture. In the case of android, the `android_arch`-parameter has to be supplied (with valid values being 'arm64v8', 'armv7' and/or 'x86'), and in the case of iOS, the `ios_arch`-parameter serves similar purposes (with valid values being 'arm64' and/or 'arm7')

Afterwards, the SContruct file found in the repository should be sufficient to build this project's C++ source code for Linux, Mac OS X, Windows and iOS (for both architectures) with the help of following command:

```
scons p=<platform> target_path=<target_path> target_name=libgdsqlite
```

In the case of Android, the [Android NDK](https://developer.android.com/ndk) needs to be installed on a Linux host to allow building for Android targets. Following command then compiles the C++ source code to all three available Android architectures at once:

```
 $ANDROID_NDK_ROOT/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk APP_PLATFORM=android-21 NDK_LIBS_OUT=<target_path>
```

For uncertainties regarding compilation & building specifics, please do check out the `.github\workflows\*.yml`-scripts, the `SConstruct`-file (for Windows, Linux, Mac OS X and iOs compilation) and both the `Android.mk`- and `jni/Application.mk`-file for the Android build process.

Tutorials for making and extending GDNative scripts are available [here](https://docs.godotengine.org/en/latest/tutorials/plugins/gdnative/gdnative-cpp-example.html)
in the Official Godot Documentation.
