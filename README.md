# godot-sqlite

This GDNative script aims to serve as a custom wrapper that makes SQLite3 available in Godot 3.1+. Additionaly, it 
does not require any additional compilation or mucking about with build scripts.

### Supported operating systems:
- Mac OS X
- Linux
- Windows

# How to use?

Re-building Godot from scratch is **NOT** required just download the build files found in the [releases](https://github.com/2shady4u/godot-sqlite/releases) tab, extract on your system and run the supplied demo-project. Make sure that the Godot is correctly loading the *gdsqlite.gdns*-resource and that it is available in the *res://*-environment.

An example project, named "demo", can also be downloaded from the releases tab. 

## Variables

- **path** (String ,default value='default')

Path to the database, should be set before opening the database with .open_db(). If no database with this name exists, a new one at the supplied path will be created. Both *res://* and *user://* keywords can be used to define the path.

- **verbose_mode** (Boolean, default=false)

Setting verbose_mode on True results in an information dump in the Godot console that is handy for debugging your ( possibly faulty) SQLite queries.

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

# How to contribute?

First clone the project and install SCons. Secondly, the C++ bindings have to be build from scratch using the files present in the godot-cpp submodule and following command:

```
scons p=<platform> bits=64 generate_bindings=yes use_custom_api_file=yes custom_api_file=../api.json
```

Afterwards, the SContruct file found in the repository should be sufficient to build this project's C++ source code for Linux, Mac OS X or Windows, with the help of following command:

```
scons p=<platform>
```

Tutorials for making and extending GDNative scripts are available [here](https://docs.godotengine.org/en/latest/tutorials/plugins/gdnative/gdnative-cpp-example.html) 
in the Official Godot Documentation. Build files are currently only available for 64-bits systems.
