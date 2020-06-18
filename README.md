![Godot SQLite banner](icon/godot-sqlite-banner.png?raw=true "Godot SQLite Banner")

# godot-sqlite

This GDNative script aims to serve as a custom wrapper that makes SQLite3 available in Godot 3.1+. Additionally, it 
does not require any additional compilation or mucking about with build scripts.

### Supported operating systems:
- Mac OS X
- Linux
- Windows

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

- Boolean success = **drop_table(** String table_name **)**

- Boolean success = **insert_row(** String table_name, Dictionary row_dictionary **)**

- Boolean success = **insert_rows(** String table_name, Array row_array **)**

- Array selected_rows = **select_rows(** String table_name, String query_conditions, Array selected_columns **)**

- Boolean success = **update_rows(** String table_name, String query_conditions, Dictionary updated_row_dictionary **)**

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
scons p=<platform> bits=64 generate_bindings=yes
```

Afterwards, the SContruct file found in the repository should be sufficient to build this project's C++ source code for Linux, Mac OS X or Windows, with the help of following command:

```
scons p=<platform>
```

Tutorials for making and extending GDNative scripts are available [here](https://docs.godotengine.org/en/latest/tutorials/plugins/gdnative/gdnative-cpp-example.html) 
in the Official Godot Documentation. Build files are currently only available for 64-bits systems.
