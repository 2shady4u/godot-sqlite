# godot-sqlite

This GDNative module aims to serve as a custom wrapper and makes SQLite3 available in Godot 3.1.

**DISCLAIMER:** There could be some stability issues until Godot 3.1 officially releases.

# How to use?

Re-building Godot from scratch is not required just download the build files found in 
the [releases](https://github.com/2shady4u/godot-sqlite/releases) tab, extract on your system and run the supplied demo-project.
Make sure that the Godot is correctly loading the *gdsqlite.gdns*-resource and that it is available in *res://*-environment.

## Variables

- **path** (String ,default value='default')

Path to the database, should be set before openening the database with .open_db()
If no database exists, a new one at the supplied path will be created. Both *res://* and *user://* keywords can be used 
to define the path.

- **verbose_mode** (Boolean, default=false)

Setting verbose_mode on True results in an information dump in the Godot console that
is handy for debugging your (faulty) SQLite queries.

- **query_result** (Array, default=[])

Contains the results from the latest query. (Only if any was generated!)

## Functions

- **open_db()**

- **close_db()**

- **query(** String query_string **)**

- **create_table(** String table_name, Dictionary table_dictionary **)**

- **drop_table(** String table_name **)**

- **insert_row(** String table_name, Dictionary row_dictionary **)**

- **insert_rows(** String table_name, Array row_array **)**

- **select_rows(** String table_name, String query_conditions, Array selected_columns **)**

- **update_rows(** String table_name, String query_conditions, Dictionary updated_row_dictionary **)**

- **delete_rows(** String table_name, String query_conditions **)**

# How to contribute?

First clone the project and install SCons. Secondly, the C++ bindings have to be build from scratch using the files present in the godot-cpp submodule and following command:

```
scons platform=<platform> bits=64 generate_bindings=yes use_custom_api_file=yes custom_api_file=../api.json
```

Afterwards, the SContruct file found in the repository should be sufficient to build this project's C++ source code to X11, OS or Windows, with the help of following command:

```
scons p = <platform>
```

Tutorials for making and extending GDNative scripts are available [here](https://docs.godotengine.org/en/latest/tutorials/plugins/gdnative/gdnative-cpp-example.html) 
in the Official Godot Documentation. Build files are currently only available for 64-bits systems.
