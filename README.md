# idaidle

Copyright 2016-2018 Google LLC

Disclaimer: This is not an official Google product (experimental or otherwise),
it is just code that happens to be owned by Google.


## What is it?

idaidle is a plugin for the commercial IDA Pro disassembler that warns users if
they leave their instance idling for too long. After a predetermined amount of
idle time, the plugin first warns and later then saves the current disassemlby
database and closes IDA.

This is useful in organizations with IDA Pro floating licenses to make sure
that analysts return their license to the license server when they are done
using it. 


## How to Build

Dependencies:
  * IDA 6.9 or higher or IDA 7.0 or higher with their respective SDKs
    installed
  * Linux/macOS: GCC/Clang with C++11 support
  * Windows: Visual Studio 2015 Compiler or later
  * CMake 3.7 or higher

First run CMake to configure the build, replacing `<IDASDK>` with the root
path of your IDA SDK installation:

*Linux/macOS*:
```bash
mkdir -p build && cd build
cmake .. -DIdaSdk_ROOT_DIR=<IDASDK> -DCMAKE_BUILD_TYPE=Release
```

*Windows*:
```dos
if not exist build mkdir build
cd build
cmake .. -G "Visual Studio 14 2015 Win64" -DIdaSdk_ROOT_DIR=<IDASDK>
```

To configure for 32-bit IDA (IDA 6.95 or lower, or IDA 7.0 "old_x86"), use
the IDA 6.95 SDK and the following commands instead:

*Linux/macOS*:
```bash
mkdir -p build && cd build
cmake .. -DIdaSdk_ROOT_DIR=<IDASDK> -DCMAKE_BUILD_TYPE=Release \
  -DBUILD_32BIT=ON
```

*Windows*:
```dos
if not exist build mkdir build
cd build
cmake .. -G "Visual Studio 14 2015" -DIdaSdk_ROOT_DIR=<IDASDK> ^
  -DBUILD_32BIT=ON
```

Once configured, start the build with:

*Linux/macOS*"
```bash
cmake --build .
```

*Windows*:
```dos
cmake --build . --config=Release
```

If all goes well, depending on your configuration, the following plugin files
are now in the build directory:

| OS      | 64-bit build (IDA 7.0+) | 32-bit build (6.x or 7.x "old_x86") |
| ------- | ----------------------- | ----------------------------------- |
| Linux   | `idaidle.so`            | `idaidle.plx`                       |
|         | `idaidle64.so`          | `idaidle.plx64`                     |
| macOS   | `idaidle.dylib`         | `idaidle.pmc`                       |
|         | `idaidle64.dylib`       | `idaidle.pmc64`                     |
| Windows | `idaidle.dll`           | `idaidle.plw`                       |
|         | `idaidle64.dll`         | `idaidle.p64`                       |

Note: A `64` in anywhere in any of the filenames denotes a 64-bit address
aware plugin.


## Installation

To install system-wide, put the plugin binaries into the `plugins` folder
in your IDA Pro installation. Below are the default paths:

| OS      | Plugin path                                 |
| ------- | ------------------------------------------- |
| Linux   | `/opt/ida-7.0/plugins`                      |
| macOS   | `/Applications/IDA Pro 7.0/idabin/plugins`  |
| Windows | `%ProgramFiles(x86)%\IDA 7.0\plugins`       |

Replace `7.0` with your actual version number.

To install just for the current user, copy the files into one of these
directories instead:

| OS          | Plugin path                          |
| ----------- | ------------------------------------ |
| Linux/macOS | `~/.idapro/plugins`                  |
| Windows     | `%AppData%\Hex-Rays\IDA Pro\plugins` |


## Usage

As soon as a database is opened, the plugin starts to monitor idle time, i.e.
the time between to consecutive UI operations. By default, after six hours,
a warning is printed to the output window. After 12 hours, a database
snapshot will be created and the IDA Pro instance will be closed without
saving. This is so that the plugin does not accidentally overwrite unsaved
work or databases the analyst did not want to save.

There is no configuration file, but the following command-line options are
available:

| Option                      | Description                                    |
| --------------------------- | ---------------------------------------------- |
| `-OIdaIdleWarningSeconds:N` | Warn the user after _N_ seconds of inactivity  |
| `-OIdaIdleTimeoutSeconds:N` | Create snapshot and close IDA afer _N_ seconds |

Note: IDA only recognizes these command-line options if they come _before_ any
filenames.
