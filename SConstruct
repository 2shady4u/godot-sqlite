#!/usr/bin/env python
import os
import sys

target_path = ARGUMENTS.pop("target_path", "demo/addons/godot-sqlite/bin/")
target_name = ARGUMENTS.pop("target_name", "libgdsqlite")

env = SConscript("godot-cpp/SConstruct")

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
sources = [Glob('src/*.cpp'), Glob('src/vfs/*.cpp'), Glob('src/resource/*.cpp'), 'src/sqlite/sqlite3.c']

if env["platform"] == "macos":
    target = "{}.{}.{}.framework/{}.{}.{}".format(
        target,
        env["platform"], 
        env["target"],
        target_name,
        env["platform"],
        env["target"]
    )
else:
    target = "{}{}{}".format(
        target,
        env["suffix"],
        env["SHLIBSUFFIX"]
    )

library = env.SharedLibrary(target=target, source=sources)
Default(library)
