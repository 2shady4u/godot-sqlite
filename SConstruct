#!/usr/bin/env python

import os
import sys
import subprocess

def add_sources(sources, dir, extension):
    for f in os.listdir(dir):
        if f.endswith('.' + extension):
            sources.append(dir + '/' + f)

if sys.version_info < (3,):
    def decode_utf8(x):
        return x
else:
    import codecs
    def decode_utf8(x):
        return codecs.utf_8_decode(x)[0]

# Workaround for MinGW. See:
# http://www.scons.org/wiki/LongCmdLinesOnWin32
if (os.name=="nt"):
    import subprocess

    def mySubProcess(cmdline,env):
        #print "SPAWNED : " + cmdline
        startupinfo = subprocess.STARTUPINFO()
        startupinfo.dwFlags |= subprocess.STARTF_USESHOWWINDOW
        proc = subprocess.Popen(cmdline, stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            stderr=subprocess.PIPE, startupinfo=startupinfo, shell = False, env = env)
        data, err = proc.communicate()
        rv = proc.wait()
        if rv:
            print("=====")
            print(err.decode("utf-8"))
            print("=====")
        return rv

    def mySpawn(sh, escape, cmd, args, env):

        newargs = ' '.join(args[1:])
        cmdline = cmd + " " + newargs

        rv=0
        if len(cmdline) > 32000 and cmd.endswith("ar") :
            cmdline = cmd + " " + args[1] + " " + args[2] + " "
            for i in range(3,len(args)) :
                rv = mySubProcess( cmdline + args[i], env )
                if rv :
                    break
        else:
            rv = mySubProcess( cmdline, env )

        return rv


def sys_exec(args):
    proc = subprocess.Popen(args, stdout=subprocess.PIPE, text=True)
    (out, err) = proc.communicate()
    return out.rstrip("\r\n").lstrip()

#################
#OPTIONS#########
#################

# Try to detect the host platform automatically.
# This is used if no `platform` argument is passed
if sys.platform.startswith('linux'):
    host_platform = 'linux'
elif sys.platform == 'darwin':
    host_platform = 'osx'
elif sys.platform == 'win32' or sys.platform == 'msys':
    host_platform = 'windows'
else:
    raise ValueError(
        'Could not detect platform automatically, please specify with '
        'platform=<platform>'
    )

opts = Variables([], ARGUMENTS)

# Gets the standard flags CC, CCX, etc.
env = DefaultEnvironment()

# Define our options
opts.Add(EnumVariable(
    'platform',
    'Target platform',
    host_platform,
    allowed_values=('linux', 'osx', 'windows', 'ios'),
    ignorecase=2
))
opts.Add(EnumVariable(
    'bits',
    'Target platform bits',
    'default',
    ('default', '32', '64')
))
opts.Add(BoolVariable(
    'use_llvm',
    'Use the LLVM compiler - only effective when targeting Linux',
    False
))
opts.Add(BoolVariable(
    'use_mingw',
    'Use the MinGW compiler instead of MSVC - only effective on Windows',
    False
))
# Must be the same setting as used for cpp_bindings
opts.Add(EnumVariable(
    'target',
    'Compilation target',
    'debug',
    allowed_values=('debug', 'release'),
    ignorecase=2
))
opts.Add(EnumVariable(
    'ios_arch',
    'Target iOS architecture',
    'arm64',
    ['armv7', 'arm64', 'x86_64']
))
opts.Add(
    'IPHONEPATH',
    'Path to iPhone toolchain',
    '/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain',
)
opts.Add(PathVariable(
    'target_path', 
    'The path where the lib is installed.', 
    'demo/bin/'
))
opts.Add(PathVariable(
    'target_name', 
    'The library name.', 
    'libgdexample', 
    PathVariable.PathAccept
))

# Local dependency paths, adapt them to your setup
godot_headers_path = "godot-cpp/godot_headers/"
cpp_bindings_path = "godot-cpp/"

env = Environment(ENV = os.environ)
# Updates the environment with the option variables.
opts.Update(env)
# Generates help for the -h scons option.
Help(opts.GenerateHelpText(env))

is64 = sys.maxsize > 2**32
if (
    env['TARGET_ARCH'] == 'amd64' or
    env['TARGET_ARCH'] == 'emt64' or
    env['TARGET_ARCH'] == 'x86_64' or
    env['TARGET_ARCH'] == 'arm64-v8a'
):
    is64 = True

# For the reference:
# - CCFLAGS are compilation flags shared between C and C++
# - CFLAGS are for C-specific compilation flags
# - CXXFLAGS are for C++-specific compilation flags
# - CPPFLAGS are for pre-processor flags
# - CPPDEFINES are for pre-processor defines
# - LINKFLAGS are for linking flags

# This makes sure to keep the session environment variables on Windows.
# This way, you can run SCons in a Visual Studio 2017 prompt and it will find
# all the required tools
if host_platform == 'windows':
    if env['bits'] == '64':
        env = Environment(TARGET_ARCH='amd64')
    elif env['bits'] == '32':
         env = Environment(TARGET_ARCH='x86')
    opts.Update(env)

if env['bits'] == 'default':
    env['bits'] = '64' if is64 else '32'

arch_suffix = env['bits']
if env['platform'] == 'ios':
    arch_suffix = env['ios_arch']

###################
####FLAGS##########
###################

if env['platform'] == 'linux':

    if env['use_llvm']:
        env['CC'] = 'clang'
        env['CXX'] = 'clang++'

    env.Append(CCFLAGS=['-fPIC'])
    env.Append(CXXFLAGS=['-std=c++17'])
    if env['target'] == 'debug':
        env.Append(CCFLAGS = ['-g3','-Og'])
    elif env['target'] == 'release':
        env.Append(CCFLAGS = ['-g','-O3'])

    if env['bits'] == '64':
        env.Append(CCFLAGS=['-m64'])
        env.Append(LINKFLAGS=['-m64'])
    elif env['bits'] == '32':
        env.Append(CCFLAGS=['-m32'])
        env.Append(LINKFLAGS=['-m32'])

elif env['platform'] == 'osx':

    # Use Clang on macOS by default
    env['CC'] = 'clang'
    env['CXX'] = 'clang++'

    if env['bits'] == '32':
        raise ValueError(
            'Only 64-bit builds are supported for the macOS target.'
        )

    env.Append(CCFLAGS=['-g', '-arch', 'x86_64'])
    env.Append(CXXFLAGS=['-std=c++17'])
    env.Append(LINKFLAGS=[
        '-arch',
        'x86_64',
        '-framework',
        'Cocoa',
        '-Wl,-undefined,dynamic_lookup',
    ])

    if env['target'] == 'debug':
        env.Append(CCFLAGS=['-Og'])
    elif env['target'] == 'release':
        env.Append(CCFLAGS=['-O3'])

elif env['platform'] == 'ios':
    env['target_path'] += env['ios_arch'] + '/'

    if env['ios_arch'] == 'x86_64':
        sdk_name = 'iphonesimulator'
        env.Append(CCFLAGS=['-mios-simulator-version-min=10.0'])
    else:
        sdk_name = 'iphoneos'
        env.Append(CCFLAGS=['-miphoneos-version-min=10.0'])

    try:
        sdk_path = decode_utf8(subprocess.check_output(['xcrun', '--sdk', sdk_name, '--show-sdk-path']).strip())
    except (subprocess.CalledProcessError, OSError):
        raise ValueError("Failed to find SDK path while running xcrun --sdk {} --show-sdk-path.".format(sdk_name))

    compiler_path = env['IPHONEPATH'] + '/usr/bin/'
    env['ENV']['PATH'] = env['IPHONEPATH'] + "/Developer/usr/bin/:" + env['ENV']['PATH']

    env['CC'] = compiler_path + 'clang'
    env['CXX'] = compiler_path + 'clang++'
    env['AR'] = compiler_path + 'ar'
    env['RANLIB'] = compiler_path + 'ranlib'

    env.Append(CCFLAGS=['-g', '-arch', env['ios_arch'], '-isysroot', sdk_path])
    env.Append(CXXFLAGS=['-std=c++17'])
    env.Append(LINKFLAGS=[
        '-arch',
        env['ios_arch'],
        '-framework',
        'Cocoa',
        '-Wl,-undefined,dynamic_lookup',
        '-isysroot', sdk_path,
        '-F' + sdk_path
    ])

    if env['target'] == 'debug':
        env.Append(CCFLAGS=['-Og'])
    elif env['target'] == 'release':
        env.Append(CCFLAGS=['-O3'])

elif env['platform'] == 'windows':

    if host_platform == 'windows' and not env['use_mingw']:
        # MSVC
        env.Append(LINKFLAGS=['/WX'])
        if env['target'] == 'debug':
            env.Append(CCFLAGS=['/Z7', '/Od', '/EHsc', '/D_DEBUG', '/MDd'])
        elif env['target'] == 'release':
            env.Append(CCFLAGS=['/O2', '/EHsc', '/DNDEBUG', '/MD'])

    elif host_platform == 'linux' or host_platform == 'osx':
        # Cross-compilation using MinGW
        if env['bits'] == '64':
            env['CC'] = 'x86_64-w64-mingw32-g'
            env['CXX'] = 'x86_64-w64-mingw32-g++'
            env['AR'] = "x86_64-w64-mingw32-ar"
            env['RANLIB'] = "x86_64-w64-mingw32-ranlib"
            env['LINK'] = "x86_64-w64-mingw32-g++"
        elif env['bits'] == '32':
            env['CC'] = 'i686-w64-mingw32-g'
            env['CXX'] = 'i686-w64-mingw32-g++'
            env['AR'] = "i686-w64-mingw32-ar"
            env['RANLIB'] = "i686-w64-mingw32-ranlib"
            env['LINK'] = "i686-w64-mingw32-g++"
    elif host_platform == 'windows' and env['use_mingw']:
        env = env.Clone(tools=['mingw'])
        env["SPAWN"] = mySpawn

    # Native or cross-compilation using MinGW
    if host_platform == 'linux' or host_platform == 'osx' or env['use_mingw']:
        env.Append(CCFLAGS=['-g', '-O3', '-Wwrite-strings'])
        env.Append(CXXFLAGS=['-std=c++17'])
        env.Append(LINKFLAGS=[
            '--static',
            '-Wl,--no-undefined',
            '-static-libgcc',
            '-static-libstdc++',
        ])

#####################
#ADD SOURCES#########
#####################
cpp_bindings_libname = 'libgodot-cpp.{}.{}.{}'.format(
                        env['platform'],
                        env['target'],
                        arch_suffix)

env.Append(CPPPATH=['.', godot_headers_path, cpp_bindings_path + 'include/', cpp_bindings_path + 'include/core/', cpp_bindings_path + 'include/gen/'])
env.Append(LIBS=[cpp_bindings_libname])
env.Append(LIBPATH=[cpp_bindings_path + 'bin/'])

# tweak this if you want to use different folders, or more folders, to store your source code in.
env.Append(CPPPATH=['src/'])
sources = [Glob('src/*.cpp'), 'src/sqlite/sqleet.c']

###############
#BUILD LIB#####
###############

# determine to link as shared or static library
if env['platform'] == "ios":
    library = env.StaticLibrary(target=env['target_path'] + env['target_name'], source=sources)
else:
    library = env.SharedLibrary(target=env['target_path'] + env['target_name'], source=sources)