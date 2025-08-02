#!/usr/bin/env python
from misc.utility.classes import CompileTimeOption

options: list = [
    CompileTimeOption(
        key="enable_fts5",
        name="FTS5",
        help="Enable SQLite's FTS5 extension which provides full-test search functionality to database applications",
        define="SQLITE_ENABLE_FTS5",
    ),
    CompileTimeOption(
        key="enable_math_functions",
        name="MATH_FUNCTIONS",
        help="Enable SQLite's Built-in Mathematical SQL Functions",
        define="SQLITE_ENABLE_MATH_FUNCTIONS",
    ),
]

target_path = ARGUMENTS.pop("target_path", "demo/addons/godot-sqlite/bin/")
target_name = ARGUMENTS.pop("target_name", "libgdsqlite")

parsed_options = {x.key: ARGUMENTS.pop(x.key) for x in options if x.key in ARGUMENTS}
env = SConscript("godot-cpp/SConstruct")
ARGUMENTS.update(parsed_options)

env_vars = Variables()
option: CompileTimeOption
[env_vars.Add(BoolVariable(x.key, x.help, False)) for x in options]
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
sources = [Glob("src/*.cpp"), Glob("src/vfs/*.cpp"), "src/sqlite/sqlite3.c"]

if env["target"] in ["editor", "template_debug"]:
    doc_data = env.GodotCPPDocData("src/gen/doc_data.gen.cpp", source=Glob("doc_classes/*.xml"))
    sources.append(doc_data)

option: CompileTimeOption
for option in options:
    if env[option.key]:
        print(f"{option.name} is enabled.")
        env.Append(CPPDEFINES=[option.define])
    else:
        print(f"{option.name} is disabled.")

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