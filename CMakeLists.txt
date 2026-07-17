# Copyright 2016-2026 Google LLC. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.31)
project(idaidle CXX)

set(CMAKE_CXX_STANDARD 20)

include(FetchContent)
FetchContent_Declare(idasdk
  URL https://github.com/HexRaysSA/ida-sdk/archive/refs/tags/v9.4.0-release.zip
  URL_HASH SHA256=b2ab453d52b3ebc22ca5da5ba2d77216335980cad8b002f47276347ab387c629
)
FetchContent_MakeAvailable(idasdk)
include(${idasdk_SOURCE_DIR}/src/cmake/idasdk_init.cmake)

find_package(idasdk REQUIRED)

# Force static runtime on Windows
set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")

ida_add_plugin(idaidle
  SOURCES idaidle.cc
)
target_compile_definitions(idaidle
  PRIVATE USE_DANGEROUS_FUNCTIONS=1
)
set_target_properties(idaidle PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
  RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
