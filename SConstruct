#!/usr/bin/env python

target_path = ARGUMENTS.pop("target_path", "demo/addons/godot-sqlite/bin/")
target_name = ARGUMENTS.pop("target_name", "libgdsqlite")

env = SConscript("godot-cpp/SConstruct")

env_vars = Variables()
env_vars.Add(BoolVariable("enable_fts5", "Enable SQLite's FTS5 extension which provides full-test search functionality to database applications", False))
env_vars.Update(env)
Help(env_vars.GenerateHelpText(env))

target = "{}{}".format(
    target_path, target_name
)

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=["src/"])
sources = [Glob('src/*.cpp'), Glob('src/vfs/*.cpp'), 'src/sqlite/sqlite3.c']

if env["target"] in ["editor", "template_debug"]:
    doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
    sources.append(doc_data)

if env["enable_fts5"]:
    print("FTS5 is enabled.")
    env.Append(CPPDEFINES=['SQLITE_ENABLE_FTS5'])
else:
    print("FTS5 is disabled.")

if env["platform"] == "macos":
    library = env.SharedLibrary(
        "{}.{}.{}.framework/{}.{}.{}".format(
            target,
            env["platform"],
            env["target"],
            target_name,
            env["platform"],
            env["target"]
        ),
        source=sources,
    )
elif env["platform"] == "ios":
    if env["ios_simulator"]:
        library = env.StaticLibrary(
            "{}.{}.{}.simulator.a".format(
                target,
                env["platform"],
                env["target"]),
            source=sources,
        )
    else:
        library = env.StaticLibrary(
            "{}.{}.{}.a".format(
                target,
                env["platform"],
                env["target"]),
            source=sources,
        )
else:
    library = env.SharedLibrary(
        "{}{}{}".format(
            target,
            env["suffix"],
            env["SHLIBSUFFIX"]
        ),
        source=sources,
    )

Default(library)