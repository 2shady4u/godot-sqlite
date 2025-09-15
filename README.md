***NOTE**: This branch is compatible with Godot 4.x. Older versions of Godot (3.x) are supported on the `godot-3.x`-branch as found [here](https://github.com/2shady4u/godot-sqlite/tree/godot-3.x).*

![Godot SQLite banner](icon/godot-sqlite-banner-4.x.png?raw=true "Godot SQLite Banner")

# godot-sqlite

This GDNative script aims to serve as a custom wrapper that makes SQLite3 available in Godot 4.0+. Additionally, it
does not require any additional compilation or mucking about with build scripts.

### Supported operating systems:
- Mac OS X (universal)
- Linux
- Windows
- Android (arm64)
- iOS (arm64)
- HTML5 (stability depends on browser)

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

It's also possible to manually download the build files found in the [releases](https://github.com/2shady4u/godot-sqlite/releases) tab, extract them on your system and run the supplied demo-project. Make sure that Godot is correctly loading the `gdsqlite.gdextension`-resource and that it is available in the `res://`-environment.

An example project, named "demo", can also be downloaded from the releases tab.

# How to use?

Examples of possible usage can be found in the supplied demo-project as downloadable in the [releases](https://github.com/2shady4u/godot-sqlite/releases) tab or the source code can be directly inspected [here](https://github.com/2shady4u/godot-sqlite/blob/master/demo/database.gd).

Additionally, a video tutorial by [Mitch McCollum (finepointcgi)](https://github.com/finepointcgi) is available here: 

<p align="center">
  <a href="https://www.youtube.com/watch?v=j-BRiTrw_F0"><img src="https://img.youtube.com/vi/j-BRiTrw_F0/0.jpg"></a>
</p>

## Variables

- **path** (String, default="default")

    Path to the database, should be set before opening the database with `open_db()`. If no database with this name exists, a new one at the supplied path will be created. Both `res://` and `user://` keywords can be used to define the path.

- **error_message** (String, default="")

    Contains the zErrMsg returned by the SQLite query in human-readable form. An empty string corresponds with the case in which the query executed succesfully.

- **default_extension** (String, default="db")

    Default extension that is automatically appended to the `path`-variable whenever **no** extension is detected/given.

    ***NOTE**: If database files without extension are desired, this variable has to be set to "" (= an empty string) as to skip this automatic procedure entirely.*

- **foreign_keys** (Boolean, default=false)

    Enables or disables the availability of [foreign keys](https://www.sqlite.org/foreignkeys.html) in the SQLite database.

- **read_only** (Boolean, default=false)

    Enabling this property opens the database in read-only modus & allows databases to be packaged inside of the PCK. To make this possible, a custom [VFS](https://www.sqlite.org/vfs.html) is employed which internally takes care of all the file handling using the Godot API.

- **query_result** (Array, default=[])

    Contains the results from the latest query **by value**; meaning that this property is safe to use when looping successive queries as it does not get overwritten by any future queries.

- **query_result_by_reference** (Array, default=[])

    Contains the results from the latest query **by reference** and is, as a direct result, cleared and repopulated after every new query.

- **last_insert_rowid** (Integer, default=0)

    Exposes the `sqlite3_last_insert_rowid()`-method to Godot as described [here](https://www.sqlite.org/c3ref/last_insert_rowid.html).  
    Attempting to modify this variable directly is forbidden and throws an error.

- **verbosity_level** (Integer, default=1)

    The verbosity_level determines the amount of logging to the Godot console that is handy for debugging your (possibly faulty) SQLite queries.

    | Level            | Description                                 |
    |----------------- | ------------------------------------------- |
    | QUIET (0)        | Don't print anything to the console         |
    | NORMAL (1)       | Print essential information to the console  |
    | VERBOSE (2)      | Print additional information to the console |
    | VERY_VERBOSE (3) | Same as VERBOSE                             |

    ***NOTE**: VERBOSE and higher levels might considerably slow down your queries due to excessive logging.*

## Methods

- Boolean success = **open_db()**

    Open a new database connection. Multiple concurrently open connections to the same database are possible.

- Boolean success = **close_db()**

    Close the current database connection.

- Boolean success = **query(** String query_string **)**

    Query the database using the raw SQL statement defined in `query_string`.

- Boolean success = **query_with_bindings(** String query_string, Array param_bindings **)**

    Binds the parameters contained in the `param_bindings`-variable to the query. Using this function stops any possible attempts at SQL data injection as the parameters are sanitized. More information regarding parameter bindings can be found [here](https://www.sqlite.org/c3ref/bind_blob.html).

    **Example usage**:

    ```gdscript
    var column_name : String = "name";
    var query_string : String = "SELECT %s FROM company WHERE age < ?;" % [column_name]
    var param_bindings : Array = [24]
    var success = db.query_with_bindings(query_string, param_bindings)
    # Executes following query: 
    # SELECT name FROM company WHERE age < 24;
    ```

    Using bindings is optional, except for PackedByteArray (= raw binary data) which has to binded to allow the insertion and selection of BLOB data in the database.

    ***NOTE**: Binding column names is not possible due to SQLite restrictions. If dynamic column names are required, insert the column name directly into the `query_string`-variable itself (see https://github.com/2shady4u/godot-sqlite/issues/41).* 

- Boolean success = **create_table(** String table_name, Dictionary table_dictionary **)**

    Each key/value pair of the `table_dictionary`-variable defines a column of the table. Each key defines the name of a column in the database, while the value is a dictionary that contains further column specifications.

    **Required fields**:

    - **"data_type"**: type of the column variable, following values are valid\*:

        | value       | SQLite         | Godot                  |
        |:-----------:|:--------------:|:----------------------:|
        | int         | INTEGER        | TYPE_INT               |
        | real        | REAL           | TYPE_REAL              |
        | text        | TEXT           | TYPE_STRING            |
        | char(?)\*\* | CHAR(?)\*\*    | TYPE_STRING            |
        | blob        | BLOB           | TYPE_PACKED_BYTE_ARRAY |

        \* *Data types not found in this table throw an error and end up finalizing the current SQLite statement.*  
        \*\* *with the question mark being replaced by the maximum amount of characters*

    **Optional fields**:

    - **"not_null"** *(default = false)*: Is the NULL value an invalid value for this column?

    - **"unique"** *(default = false)*: Does the column have a unique constraint?

    - **"default"**: The default value of the column if not explicitly given.

    - **"primary_key"** *(default = false)*: Is this the primary key of this table?  
    Multiple columns can be set as a primary key.

    - **"auto_increment"** *(default = false)*: Automatically increment this column when no explicit value is given. This auto-generated value will be one more (+1) than the largest value currently in use.

        ***NOTE**: Auto-incrementing a column only works when this column is the primary key and no other columns are primary keys!*

    - **"foreign_key"**: Enforce an "exist" relationship between tables by setting this variable to `foreign_table.foreign_column`. In other words, when adding an additional row, the column value should be an existing value as found in the column with name `foreign_column` of the table with name `foreign_table`.

        ***NOTE**: Availability of foreign keys has to be enabled by setting the `foreign_keys`-variable to true BEFORE opening the database.*

    **Example usage**:

    ```gdscript
    # Add the row "id" to the table, which is an auto-incremented primary key.
    # When adding additional rows, this value can either by explicitely given or be unfilled.
    table_dictionary["id"] = {
        "data_type": "int", 
        "primary_key": true, 
        "auto_increment": true
    }
    ```

    For more concrete usage examples see the `database.gd`-file as found in this repository's demo project.

- Boolean success = **drop_table(** String table_name **)**

    Drop the table with name `table_name`. This method is equivalent to the following query:
    ```
    db.query("DROP TABLE "+ table_name + ";")
    ```

- Boolean success = **insert_row(** String table_name, Dictionary row_dictionary **)**

    Each key/value pair of the `row_dictionary`-variable defines the column values of a single row.  

    Columns should adhere to the table schema as instantiated using the `table_dictionary`-variable and are required if their corresponding **"not_null"**-column value is set to `True`.

- Boolean success = **insert_rows(** String table_name, Array row_array **)**

- Array selected_rows = **select_rows(** String table_name, String query_conditions, Array selected_columns **)**

    Returns the results from the latest query **by value**; meaning that this property does not get overwritten by any successive queries.

- Boolean success = **update_rows(** String table_name, String query_conditions, Dictionary updated_row_dictionary **)**

    With the `updated_row_dictionary`-variable adhering to the same table schema & conditions as the `row_dictionary`-variable defined previously.

- Boolean success = **delete_rows(** String table_name, String query_conditions **)**

- Boolean success = **import_from_json(** String import_path **)**

    Drops all database tables and imports the database structure and content present inside of `import_path.json`.

- Boolean success = **export_to_json(** String export_path **)**

    Exports the database structure and content to `export_path.json` as a backup or for ease of editing.

- Boolean success = **import_from_buffer(** PackedByteArray input_buffer **)**

    Drops all database tables and imports the database structure and content encoded in JSON-formatted input_buffer.

    Can be used together with `export_to_buffer()` to implement database encryption.

- PackedByteArray output_buffer = **export_to_buffer()**

    Returns the database structure and content as JSON-formatted buffer. 

    Can be used together with `import_from_buffer()` to implement database encryption.

- Boolean success = **create_function(** String function_name, FuncRef function_reference, int number_of_arguments **)**

    Bind a [scalar SQL function](https://www.sqlite.org/appfunc.html) to the database that can then be used in subsequent queries.

- int autocommit_mode = **get_autocommit()**

    Check if the given database connection is or is not in autocommit mode, see [here](https://sqlite.org/c3ref/get_autocommit.html).

- int compileoption_used = **compileoption_used(** String option_name **)**

    Check if the binary was compiled using the specified option, see [here](https://sqlite.org/c3ref/compileoption_get.html).

    Mostly relevant for checking if the [SQLite FTS5 Extension](https://sqlite.org/fts5.html) is enabled, in which case the following lines can be used:

    ```gdscript
    db.compileoption_used("SQLITE_ENABLE_FTS5") # Returns '1' if enabled or '0' if disabled
    db.compileoption_used("ENABLE_FTS5") # The "SQLITE_"-prefix may be omitted.
    ```

- Boolean success = **backup_to(** String destination_path **)**
- Boolean success = **restore_from(** String source_path **)**

    Backup or restore the current database to/from a path, see [here](https://www.sqlite.org/backup.html). This feature is useful if you are using a database as your save file and you want to easily implement a saving/loading mechanic. Be warned that the original database will be overwritten entirely when restoring.

- int enable_load_extension = **enable_load_extension(** Boolean onoff **)**

    [Extension loading](https://www.sqlite.org/c3ref/load_extension.html) is disabled by default for security reasons. There are two ways to load an extension: C-API and SQL function. This method turns on both options.
    SQL function `load_extension()` can only be used after enabling extension loading with this method. Preferably should be disabled after loading the extension to prevent SQL injections. Returns the SQLite return code.

    ```gdscript
    var module_path = ProjectSettings.globalize_path("res://addons/godot-sqlite/extensions/spellfix.dll")
    db.enable_load_extension(true)
    db.query_with_bindings(
        "select load_extension(?, ?);", [
            module_path,
            "sqlite3_spellfix_init"
        ])
    db.enable_load_extension(false)
    ```

- int load_extension = **load_extension(** String extension_path, String extension_entry_point **)**

    Loads the extension in the given path. Does not require `enable_load_extension()`, as it only enables C-API during the call and disables it right after, utilizing the recommended extension loading method declared by the SQLite documentation ([see](https://www.sqlite.org/c3ref/load_extension.html)). Returns the SQLite return code.
    - **extension_path:** the path to the compiled binary of the extension
    - **entrypoint:** the extension's entrypoint method (init function). It is defined in the .c file of the extension.

   Example for loading the spellfix module:

    ```gdscript
    db.load_extension("res://addons/godot-sqlite/extensions/spellfix.dll", "sqlite3_spellfix_init")
    ```

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

### 2. When should I create function bindings to augment SQLite's set of native functions?

Preferably never.

Creating function should only be seen as a measure of last resort and only be used when you perfectly know what you are doing. Be sure to first check out the available native list of [scalar SQL applications](https://www.sqlite.org/lang_corefunc.html) that is already available in SQLite3.

### 3. My Android/iOS/HTML5 application cannot access the database!

Android does not allow modification of files in the `res://`-folder, thus blocking the plugin from acquiring a read and write lock on the database-file.

In the case of read-only databases, the database files have to be packaged/exported in the `*.pck`-file by manually adding the file entry to the `include_filter` of your `export_presets.cfg`-file.
Additionally the connection has to be opened by explicitly setting the `read_only`-variable of your connection to True before opening your database.

In the case of a read and write database connection, the most painless solution is to copy the entire database to the `user://`-folder instead as apps have explicit writing privileges there.
If there is a better solution, one that does not involve copying the database to a new location, please do enlighten me.

### 4. Is this plugin compatible with a Godot Server binary? How to set it up?

Follow the steps described in the Godot documentation as found [here](https://docs.godotengine.org/en/stable/tutorials/export/exporting_for_dedicated_servers.html).

***NOTE**: If you are using an older version of Linux on your server machine (with glibc version < 2.35), the plugin crashes due to the compiled version of glibc being too recent. In that case you will have to recompile the Linux plugin binary yourself as Github Actions does not support Ubuntu versions lower than Ubuntu 22.04 LTS.*

***NOTE**: The currently installed version of glibc can be obtained with following command:*
```
ldd --version
```

### 5. Does this plugin support the XYZ Extension of SQLite?

Following SQLite extensions are supported by this plugin, although they require the user to re-compile the plugin:

| Extension                                                                    | flag                  | default |
|------------------------------------------------------------------------------|-----------------------|---------|
| [SQLite FTS5 Extension](https://sqlite.org/fts5.html)                        | enable_fts5           | no      |
| [Built-In Mathematical SQL Functions](https://sqlite.org/lang_mathfunc.html) | enable_math_functions | no      |

To re-compile the plugin with XYZ enabled, follow the instructions as defined in the 'How to contribute?'-section below.  
Depending on your choice, following modifications have to be made:

#### A. Using your own device

Add the relevant flag(s), as defined in table above, to the compilation command.  
For example, if you want to enable the FTS5 Extension, use following command:

```
scons platform=<platform> target_path=<target_path> target_name=libgdsqlite enable_fts5=yes
```

#### B. Using Github Actions

Update the `common_flags`-field of the `.github/workflows/build_var.json`-file, as follows:

```json
"common_flags": "enable_fts5=no"
```

```json
"common_flags": "enable_fts5=yes"
```

Afterwards, push a new commit (containing this change) to the `master`-branch of your forked remote. This commit triggers a new workflow that re-compiles the plugin's binaries with FTS5 enabled.

### 6. Does this plugin support some kind of encryption?

Database encryption of any kind is **not** supported in this plugin. Nor are there any plans to support this feature in an upcoming or future release.

The addition of database encryption might be up for reconsideration if, and only if, a future release of SQLite introduces native support of this feature without requiring the purchase of a license.

A modification of this plugin, which allows encryption, was done by [abcjjy](https://github.com/abcjjy) by swapping out SQLite with [SQLCipher](https://www.zetetic.net/sqlcipher/) and can be found [here](https://github.com/abcjjy/godot-sqlcipher/tree/gd-extension). Direct all issues and feature requests regarding this implementation to its own dedicated repository.

***NOTE**: The natively supported [SQLite Encryption Extension (SEE)](https://sqlite.org/com/see.html) is not applicable as it requires the purchase of a license for the one-time fee of 2000$*

# How to export?

The exporting strategy is dependent on the nature of your database.

## Read-only databases

If your database serves as static data storage i.e. there's no persistent dynamic data that needs to be saved, then your database is a read-only database. For example, a database that contains a table of monster descriptions and data (experience gained on kill, health points) probably never changes during normal gameplay.

In this case, the database can be packaged in the `*.pck`-file together with all other assets without any hassle.  
To enable this behaviour following conditions need to be met:
- The database file has to be added manually to the `include_filter` of your `export_presets.cfg`-file to package the file on export.
- The connection has to be opened in read-only mode by setting the `read_only` variable to True.

You can also open databases in read-only mode that are not packaged, albeit under some restrictions such as the fact that the database files have to copied manually to `user://`-folder on mobile platforms (Android & iOS) and for web builds.

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
godot --export-debug "Windows Desktop" "build/SQLite Demo.exe"
```
("Windows Desktop" should be replaced by whatever the name of your relevant build template is.)

For the "Windows Desktop" export target this results in following folder/file-structure:
```
data/test_backup_base64_old.json
data/test_backup_old.json
libgdsqlite.windows.template_release.x86_64.dll
'SQLite Demo.console.exe'
'SQLite Demo.exe'
'SQlite Demo.pck'
```

# How to contribute?

## Using GitHub Actions CI/CD

Fork the repository to your GitHub own account by following the recommended [procedure](https://docs.github.com/en/get-started/quickstart/fork-a-repo).

Afterwards you can push your own code to the `master`-branch of this forked repository and [GitHub Actions](https://docs.github.com/en/actions/quickstart) will automatically build new versions for all the supported platforms. These binaries then have to be downloaded and placed inside of the `addons/godot-sqlite/bin/`-folder, as found in your Godot project, alongside any existing binaries.

***NOTE**: Compiled binaries for the latest development version of `godot-sqlite` are available [here](https://github.com/2shady4u/godot-sqlite/actions/workflows/build.yml).*

## Using your own device

First clone the project and initialize the `godot-cpp`-submodule as such:
```
git clone https://github.com/2shady4u/godot-sqlite.git
git submodule update --init
```

Secondly, [SCons](https://scons.org/), Python and a viable C++ compiler have to be installed on your device. On Windows, both Visual Studio Community (2017+) and/or MinGW-w64 can be used to compile the plugin from scratch. More detailed instructions for compiling the plugin on Windows, Linux and other supported platforms can be found [here](https://docs.godotengine.org/en/stable/development/compiling/index.html) in the Official Godot documentation.

Depending on the wanted target platform, some additional dependencies have to be installed:
- In the case of Android, the [Android NDK](https://developer.android.com/ndk) needs to be installed on a Linux host to allow building for Android targets.

Afterwards, the SContruct file found in this repository should be sufficient to build this project's C++ source code for Linux, Mac&nbsp;OS&nbsp;X, Windows, iOS and android with the help of following command:

```
scons platform=<platform> target_path=<target_path> target_name=libgdsqlite
```

In the case of both Android and iOS, an additional `arch`-parameter has to be supplied to specify the CPU architecture. In the case of android, valid values for the architecture are 'arm64' (= default), 'arm32', 'x86_64' and/or 'x86_32', and in the case of iOS, the valid values are 'universal' (= default), 'arm64' and/or 'x86_64'.

Additionally, in the case of Android, the `ANDROID_NDK_ROOT`-parameter has to be set to the location of the Android NDK installed previously.

For uncertainties regarding compilation & building specifics, please do check out the `.github\workflows\*.yml`-scripts and the `SConstruct`-file as found in this repository.

Tutorials for making and extending GDExtension scripts are available [here](https://docs.godotengine.org/en/stable/tutorials/scripting/gdextension/gdextension_cpp_example.html)
in the Official Godot Documentation.
