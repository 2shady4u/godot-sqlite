***NOTE**: The Godot 4.0 version of this plugin is currently in development on the `gd-extension`-branch as found [here](https://github.com/2shady4u/godot-sqlite/tree/gd-extension).*

![Godot SQLite banner](icon/godot-sqlite-banner-v2.png?raw=true "Godot SQLite Banner")

# godot-sqlite

This GDNative script aims to serve as a custom wrapper that makes SQLite3 available in Godot 3.2+. Additionally, it
does not require any additional compilation or mucking about with build scripts.

### Supported operating systems:
- Mac OS X
- Linux
- Windows
- Android (arm64-v8a, armeabi-v7a & x86)
- iOS (arm64 & armv7)
- HTML5 (**requires Godot 3.3+**)

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

It's also possible to manually download the build files found in the [releases](https://github.com/2shady4u/godot-sqlite/releases) tab, extract them on your system and run the supplied demo-project. Make sure that Godot is correctly loading the `gdsqlite.gdns`-resource and that it is available in the `res://`-environment.

An example project, named "demo", can also be downloaded from the releases tab.

# How to use?

Examples of possible usage can be found in the supplied demo-project as downloadable in the [releases](https://github.com/2shady4u/godot-sqlite/releases) tab or the source code can be directly inspected [here](https://github.com/2shady4u/godot-sqlite/blob/master/demo/database.gd).

Additionally, a video tutorial by [Mitch McCollum (finepointcgi)](https://github.com/finepointcgi) is available here: 

<p align="center">
  <a href="https://www.youtube.com/watch?v=HG-PV4rlzoY"><img src="https://img.youtube.com/vi/HG-PV4rlzoY/0.jpg"></a>
</p>

## Variables

- **path** (String, default="default")

    Path to the database, should be set before opening the database with `open_db()`. If no database with this name exists, a new one at the supplied path will be created. Both `res://` and `user://` keywords can be used to define the path.

- **error_message** (String, default="")

    Contains the zErrMsg returned by the SQLite query in human-readable form. An empty string corresponds with the case in which the query executed succesfully.

- **default_extension** (String, default="db")

    Default extension that is automatically appended to the `path`-variable whenever **no** extension is detected/given.

    ***NOTE:** If database files without extension are desired, this variable has to be set to "" (= an empty string) as to skip this automatic procedure entirely.*

- **verbose_mode** (Boolean, default=false) **[DEPRECATED]**

    Setting verbose_mode on True results in an information dump in the Godot console that is handy for debugging your (possibly faulty) SQLite queries.

    ***NOTE:** This variable will be removed in later versions and is preserved only for the sake of backwards compatibility. See the new `verbosity_level`-variable below for the recommended way to control console logging.*

- **foreign_keys** (Boolean, default=false)

    Enables or disables the availability of [foreign keys](https://www.sqlite.org/foreignkeys.html) in the SQLite database.

- **read_only** (Boolean, default=false)

    Enabling this property opens the database in read-only modus & allows databases to be packaged inside of the PCK. To make this possible, a custom [VFS](https://www.sqlite.org/vfs.html) is employed which internally takes care of all the file handling using the Godot API.

    ***NOTE:** Godot opens files in a mode that is not shareable i.e. the database file cannot be open in any other program. Attempting to open a read-only database that is locked by another program fails and returns `ERR_FILE_CANT_OPEN` (`12`). However, multiple simultaneous read-only database connections are allowed.*

- **query_result** (Array, default=[])

    Contains the results from the latest query **by value**; meaning that this property is safe to use when looping successive queries as it does not get overwritten by any future queries.

- **query_result_by_reference** (Array, default=[])

    Contains the results from the latest query **by reference** and is, as a direct result, cleared and repopulated after every new query.

- **last_insert_rowid** (Integer, default=0)

    Exposes both the `sqlite3_last_insert_rowid()`- and `sqlite3_set_last_insert_rowid()`-methods to Godot as described [here](https://www.sqlite.org/c3ref/last_insert_rowid.html) and [here](https://www.sqlite.org/c3ref/set_last_insert_rowid.html) respectively.

- **verbosity_level** (Integer, default=1)

    The verbosity_level determines the amount of logging to the Godot console that is handy for debugging your (possibly faulty) SQLite queries.

    | Level            | Description                                 |
    |----------------- | ------------------------------------------- |
    | QUIET (0)        | Don't print anything to the console         |
    | NORMAL (1)       | Print essential information to the console  |
    | VERBOSE (2)      | Print additional information to the console |
    | VERY_VERBOSE (3) | Same as VERBOSE                             |

    ***NOTE:** VERBOSE and higher levels might considerably slow down your queries due to excessive logging.*

## Functions

- Boolean success = **open_db()**

- **close_db()**

- Boolean success = **query(** String query_string **)**

- Boolean success = **query_with_bindings(** String query_string, Array param_bindings **)**

    Binds the parameters contained in the `param_bindings`-variable to the query. Using this function stops any possible attempts at SQL data injection as the parameters are sanitized. More information regarding parameter bindings can be found [here](https://www.sqlite.org/c3ref/bind_blob.html).

    **Example usage**:

    ```gdscript
    var query_string : String = "SELECT ? FROM company WHERE age < ?;"
    var param_bindings : Array = ["name", 24]
    var success = db.query_with_bindings(query_string, param_bindings)
    # Executes following query: 
    # SELECT name FROM company WHERE age < 24;
    ```

    Using bindings is optional, except for PoolByteArray (= raw binary data) which has to binded to allow the insertion and selection of BLOB data in the database.

    ***NOTE**: Binding column names is not possible due to SQLite restrictions. If dynamic column names are required, insert the column name directly into the `query_string`-variable itself (see https://github.com/2shady4u/godot-sqlite/issues/41).* 

- Boolean success = **create_table(** String table_name, Dictionary table_dictionary **)**

    Each key/value pair of the `table_dictionary`-variable defines a column of the table. Each key defines the name of a column in the database, while the value is a dictionary that contains further column specifications.

    **Required fields**:

    - **"data_type"**: type of the column variable, following values are valid\*:

        | value       | SQLite         | Godot          |
        |:-----------:|:--------------:|:--------------:|
        | int         | INTEGER        | TYPE_INT       |
        | real        | REAL           | TYPE_REAL      |
        | text        | TEXT           | TYPE_STRING    |
        | char(?)\*\* | CHAR(?)\*\*    | TYPE_STRING    |
        | blob        | BLOB           | TYPE_RAW_ARRAY |

        \* *Data types not found in this table throw an error and end up finalizing the current SQLite statement.*  
        \*\* *with the question mark being replaced by the maximum amount of characters*

    **Optional fields**:

    - **"not_null"** *(default = false)*: Is the NULL value an invalid value for this column?
    
    - **"unique"** *(default = false)*: Does the column have a unique constraint?

    - **"default"**: The default value of the column if not explicitly given.

    - **"primary_key"** *(default = false)*: Is this the primary key of this table?  
    Evidently, only a single column can be set as the primary key.

    - **"auto_increment"** *(default = false)*: Automatically increment this column when no explicit value is given. This auto-generated value will be one more (+1) than the largest value currently in use.

        ***NOTE**: Auto-incrementing a column only works when this column is the primary key!*

    - **"foreign_key"**: Enforce an "exist" relationship between tables by setting this variable to `foreign_table.foreign_column`. In other words, when adding an additional row, the column value should be an existing value as found in the column with name `foreign_column` of the table with name `foreign_table`.

        ***NOTE**: Availability of foreign keys has to be enabled by setting the `foreign_keys`-variable to true BEFORE opening the database.*

    **Example usage**:

    ```gdscript
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

    With the `updated_row_dictionary`-variable adhering to the same table schema & conditions as the `row_dictionary`-variable defined previously.

- Boolean success = **delete_rows(** String table_name, String query_conditions **)**

- Boolean success = **import_from_json(** String import_path **)**

    Drops all database tables and imports the database structure and content present inside of `import_path.json`.

- Boolean success = **export_to_json(** String export_path **)**

    Exports the database structure and content to `export_path.json` as a backup or for ease of editing.

- Boolean success = **create_function(** String function_name, FuncRef function_reference, int number_of_arguments **)**

    Bind a [scalar SQL function](https://www.sqlite.org/appfunc.html) to the database that can then be used in subsequent queries.

- Integer mode = **get_autocommit()**

    Get the current autocommit mode of the open database connection which can be used to check if there's any database transactions in progress, see [here](http://www.sqlite.org/c3ref/get_autocommit.html). A non-zero return value indicates that the database is in autocommit mode and thus has no active transaction.

## Frequently Asked Questions (FAQ)

### 1. My query fails and returns syntax errors, what should I do?

There are a couple of things you can do before panicking, namely:
- Test out if your query is valid by trying it out online at https://sqliteonline.com/.
- Use the **query(** **)** or **query_with_bindings(** **)**-function instead of the more specialized wrapper function.
- Your query might be missing some quotation marks. For example, following queries will fail due to missing encapsulation of the `default`-field:

    ```gdscript
    var table_name := "characters"
    var table_dict : Dictionary
    table_dict["last_name"] = {"data_type":"text", "default": "Silver"}
    table_dict["first_name"] = {"data_type":"text", "default": "Long John"}
    table_dict["area"] = {"data_type":"text", "default": ""}
    table_dict["color"] = {"data_type":"text", "default": "0,0,0,0"}
    db.create_table(table_name, table_dict)
    ```

    Adding some well-placed single quotation marks fixes this issue:

    ```gdscript
    var table_name := "characters"
    var table_dict : Dictionary
    table_dict["last_name"] = {"data_type":"text", "default": "Silver"}
    table_dict["first_name"] = {"data_type":"text", "default": "'Long John'"}
    table_dict["area"] = {"data_type":"text", "default": "''"}
    table_dict["color"] = {"data_type":"text", "default": "'0,0,0,0'"}
    db.create_table(table_name, table_dict)
    ```

    Basically you'll need to use single quotation marks whenever:
    - The string is empty
    - The string contains syntax restricted symbols such as commas or spaces

- SQLite restricts dynamically binding the names of tables and columns, thus following query will fail due to syntax errors:
    ```gdscript
    var table_name := "characters"
    var column_name := "level"
    db.query_with_bindings("UPDATE ? SET ?=? WHERE id=?;", [table_name, column_name, 100, 1])
    ```

    This is forbidden SQLite syntax as both the `table_name`- and `column_name`-variables cannot be bound! If dynamic modification of names of tables and columns is required for purposes of your code, then use following work-around:

    ```gdscript
    var table_name := "characters"
    var column_name := "level"
    db.query_with_bindings("UPDATE "+ table_name +" SET "+ column_name +"=? WHERE id=?;", [100, 1])
    ```

After exhausting these options, please open an issue that describes the error in proper detail.

### 2. Your plugin fails to load on my Windows machine!

***NOTE**: The cause of this issue has been addressed in later releases of this plugin (3.1+) by swapping out the windows library with the MinGW cross-compiled version. This QA entry is preserved as a historical reference.* 

Basically if your Windows machine device doesn't have the required VC++ redistributables installed, the dynamic library will fail to load and throw an error of the following sort:

```
ERROR: GDNative::get_symbol: No valid library handle, can't get symbol from GDNative object
At: modules\gdnative\gdnative.cpp:315
ERROR: NativeScriptLanguage::init_library: No nativescript_init in "res://addons/godot-sqlite/bin/win64/libgdsqlite.dll" found
At: modules\gdnative\nativescript\nativescript.cpp:1054
```

This is an open issue that is still under consideration (see https://github.com/2shady4u/godot-sqlite/issues/33).

Some possible solutions/work-arounds exist:
- Install the missing VC++ redistributables (downloadable [here](https://support.microsoft.com/en-us/topic/the-latest-supported-visual-c-downloads-2647da03-1eea-4433-9aff-95f26a218cc0))
- Recompile the plugin using the MinGW compiler instead (**WARNING:** currently results in a >15MB library).
- Recompile the plugin (and the bindings) using the `/MT`-flag instead of the `/MD`-flag as discussed [here](https://docs.microsoft.com/en-us/cpp/build/reference/md-mt-ld-use-run-time-library?view=msvc-160).

If the console error is of an entirely different nature, please open an issue.

### 3. When should I create function bindings to augment SQLite's set of native functions?

Preferably never.

Creating function should only be seen as a measure of last resort and only be used when you perfectly know what you are doing. Be sure to first check out the available native list of [scalar SQL applications](https://www.sqlite.org/lang_corefunc.html) that is already available in SQLite3.

### 4. My Android/iOS/HTML5 application cannot access the database!

Android does not allow modification of files in the `res://`-folder, thus blocking the plugin from acquiring a read and write lock on the database-file.

In the case of read-only databases, the database files have to be packaged/exported in the `*.pck`-file by manually adding the file entry to the `include_filter` of your `export_presets.cfg`-file.
Additionally the connection has to be opened by explicitly setting the `read_only`-variable of your connection to True before opening your database.

In the case of a read and write database connection, the most painless solution is to copy the entire database to the `user://`-folder instead as apps have explicit writing privileges there.
If there is a better solution, one that does not involve copying the database to a new location, please do enlighten me.

### 5. Is this plugin compatible with a Godot Server binary? How to set it up?

This plugin is fully compatible with the Godot Server binary.  
Follow these steps to create a working Linux Server for your project:

1. Export your project's `*.pck` using Godot's export functionalities for Linux.
2. Alongside the exported package, paste the following files:
    - `libgdsqlite.so` (as found in `addons/godot-sqlite/bin/x11/`)
    - Your project's database(s) (`*.db`)
    - The Godot Server binary as downloadable [here](https://godotengine.org/download/server)
3. Rename the Godot Server binary to have the exact same name as the exported `*.pck`  
(for example if your package is called `game.pck`, your binary should be named `game.x64`)
4. Done!

***NOTE**: If you are using an older version of Linux on your server machine (with glibc version < 2.28), the plugin crashes due to the compiled version of glibc being too recent. In that case you can either recompile the Linux plugin binary yourself or you can download the legacy binaries (Ubuntu 16.04 with glibc version == 2.23) as found [here](https://github.com/2shady4u/godot-sqlite/actions/workflows/linux_builds.yml).* 

### 6. How to export to iOS using Xcode?

Apple disallows the usage of dynamic libraries in any and all user applications. As such, the compiled binaries for iOS are static libraries (`*.a`) and **both** the static godot-sqlite libraries (`libgdsqlite.a`) as the godot-cpp static libraries <nobr>(`libgodot-cpp.ios.release.<ios_arch>.a`</nobr>) need to be properly included in your Xcode-project.

# How to export?

The exporting strategy is dependent on the nature of your database.

## Read-only databases

If your database serves as static data storage i.e. there's no persistent dynamic data that needs to be saved, then your database is a read-only database. For example, a database that contains a table of monster descriptions and data (experience gained on kill, health points) probably never changes during normal gameplay.

In this case, the database can be packaged in the `*.pck`-file together with all other assets without any hassle.  
To enable this behaviour following conditions need to be met:
- The database file has to be added manually to the `include_filter` of your `export_presets.cfg`-file to package the file on export.
- The connection has to be opened in read-only mode by setting the `read_only` variable to True.

You can also open databases in read-only mode that are not packaged, albeit under some restrictions such as the fact that the database files have to copied manually to `user://`-folder on mobile platforms (Android & iOS) and for web builds.

One important additional constraint for read-only databases is that Godot's implementation of file handling does not allow files to opened in a shareable manner. Basically this means that opening a database connection fails whenever other programs have a read lock on the database file e.g. having the file open in [SQLiteStudio](https://sqlitestudio.pl/) for editing purposes. However, multiple simultaneous read-only database connections are allowed.

***NOTE**: The contents of your PCK file can be verified by using externally available tools as found [here](https://github.com/hhyyrylainen/GodotPckTool).*

## Read and write databases

***NOTE**: On mobile platforms (Android & iOS) and for web builds, the method discussed here is not possible and the contents of the `res://data/`-folder have to be copied to the `user://`-folder in its entirety instead (see FAQ above).*

If your database serves as dynamic data storage i.e. there's persistent dynamic data that needs to be saved during gameplay, then your database is a read and write database. For example, a database that contains a table of character levels and experience dynamically changes whenever the player levels up and/or gains experience.

In this case, the database cannot be packaged in the `*.pck`-file as the contents of this file are static and cannot be dynamically modified during normal operation.  

All `*.db`-files (and `*.json`-files if you choose not to include them in the `*.pck`) should preferably be part of the same folder. For example, in the case of the demo-project this is the `data/`-folder. During export this folder has to be copied in its entirety and placed alongside the executable that is created by Godot's export command line utilities.  
To simplify and automate this process, a script with the name `export_data.gd` can be found in the demo-project and allows to automatically copy the `data`-folder's contents to the export folder.

The commands to succesfully export a working executable for the demo-project are:
```
mkdir build
godot -s export_data.gd
godot -e --export-debug "Windows Desktop" 'build/SQLite Demo.exe'
```
("Windows Desktop" should be replaced by whatever the name of your relevant build template is.)

For the "Windows Desktop" export target this results in following folder/file-structure:
```
data/test_backup_base64_old.json
data/test_backup_old.json
libgdsqlite.dll
'SQLite Demo.exe'
'SQlite Demo.pck'
```

# How to contribute?

First clone the project and install SCons. Secondly, the C++ bindings have to be build from scratch using the files present in the godot-cpp submodule and following command:

```
scons p=<platform> bits=64 generate_bindings=yes -j4
```

In the case of Android and iOS, additional parameters have to be supplied to specify the architecture. In the case of android, the `android_arch`-parameter has to be supplied (with valid values being 'arm64v8', 'armv7' and/or 'x86'), and in the case of iOS, the `ios_arch`-parameter serves similar purposes (with valid values being 'arm64' and/or 'arm7')

Afterwards, the SContruct file found in the repository should be sufficient to build this project's C++ source code for Linux, Mac&nbsp;OS&nbsp;X, Windows, iOS (for both architectures) and HTML5 with the help of following command:

```
scons p=<platform> target_path=<target_path> target_name=libgdsqlite
```

In the case of Android, the [Android NDK](https://developer.android.com/ndk) needs to be installed on a Linux host to allow building for Android targets. Following command then compiles the C++ source code to all three available Android architectures at once:

```
 $ANDROID_NDK_ROOT/ndk-build NDK_PROJECT_PATH=. APP_BUILD_SCRIPT=Android.mk APP_PLATFORM=android-21 NDK_LIBS_OUT=<target_path>
```

For uncertainties regarding compilation & building specifics, please do check out the `.github\workflows\*.yml`-scripts, the `SConstruct`-file (for Windows, Linux, Mac OS X, iOS and HTML5 compilation) and both the `Android.mk`- and `jni/Application.mk`-files for the Android build process.

Tutorials for making and extending GDNative scripts are available [here](https://docs.godotengine.org/en/stable/tutorials/plugins/gdnative/gdnative-cpp-example.html)
in the Official Godot Documentation.
