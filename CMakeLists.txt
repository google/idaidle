# Copyright 2016-2018 Google LLC. All Rights Reserved.
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

cmake_minimum_required(VERSION 3.7)
project(idaidle CXX)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR})

option(BUILD_32BIT "Compile the plugin as a 32-bit binary" OFF)
if(BUILD_32BIT)
  set(IdaSdk_COMPILE_32BIT ON)
  set(IdaSdk_LEGACY_FILE_EXTENSIONS ON)
else()
  set(_idaidle_ida64 IDA64)
endif()

find_package(IdaSdk REQUIRED)

if(MSVC)
  # Use the static runtime
  foreach(flag_var CMAKE_CXX_FLAGS
                   CMAKE_CXX_FLAGS_DEBUG
                   CMAKE_CXX_FLAGS_RELEASE
                   CMAKE_CXX_FLAGS_MINSIZEREL
                   CMAKE_CXX_FLAGS_RELWITHDEBINFO)
    if(${flag_var} MATCHES "/MD")
      string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
    endif()
    if(${flag_var} MATCHES "/MDd")
      string(REGEX REPLACE "/MDd" "/MTd" ${flag_var} "${${flag_var}}")
    endif()
  endforeach()
endif()

add_ida_plugin(idaidle EA64 idaidle.cc)
foreach(target idaidle${_plx} idaidle${_plx64})
  set_property(TARGET ${target} PROPERTY CXX_STANDARD 11)
  target_include_directories(${target} PRIVATE ${IdaSdk_INCLUDE_DIRS})
  target_compile_definitions(${target} PRIVATE ${_idaidle_ida64})
endforeach()
