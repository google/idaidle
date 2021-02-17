# idaidle

Copyright 2016-2021 Google LLC

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
  * IDA 7.0 or higher with a matching SDK installed
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
cmake .. -DIdaSdk_ROOT_DIR=<IDASDK> -DCMAKE_BUILD_TYPE=Release
```

Once configured, start the build with:

*Linux*:
```bash
cmake --build .
```

*macOS/Windows*:
```dos
cmake --build . --config=Release
```

If all goes well, depending on your configuration, the following plugin files
are now in the build directory:

| OS      | Filename                |
| ------- | ----------------------- |
| Linux   | `idaidle.so`            |
|         | `idaidle64.so`          |
| macOS   | `idaidle.dylib`         |
|         | `idaidle64.dylib`       |
| Windows | `idaidle.dll`           |
|         | `idaidle64.dll`         |

Note: A `64` in any of the filenames denotes a 64-bit address aware plugin.


## Installation

To install system-wide, put the plugin binaries into the `plugins` folder
in your IDA Pro installation. Below are the default paths:

| OS      | Plugin path                                 |
| ------- | ------------------------------------------- |
| Linux   | `/opt/ida-7.5/plugins`                      |
| macOS   | `/Applications/IDA Pro 7.5/idabin/plugins`  |
| Windows | `%ProgramFiles(x86)%\IDA 7.5\plugins`       |

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
