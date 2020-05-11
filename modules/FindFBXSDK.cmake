# Copyright (c) 2014-present, Facebook, Inc.
# All rights reserved.
#
# Helper function for finding the FBX SDK.
# Cribbed & tweaked from https://github.com/facebookincubator/FBX2glTF
#
# params: FBXSDK_VERSION
#         FBXSDK_SDKS
#
# sets:   FBXSDK_FOUND
#         FBXSDK_DIR
#         FBXSDK_LIBRARY
#         FBXSDK_LIBRARY_DEBUG
#         FBXSDK_INCLUDE_DIR
#

# semi-hack to detect architecture
if( CMAKE_SIZEOF_VOID_P MATCHES 8 )
  # void ptr = 8 byte --> x86_64
  set(ARCH_32 OFF)
else()
  # void ptr != 8 byte --> x86
  set(ARCH_32 OFF)
endif()

if (NOT DEFINED FBXSDK_VERSION)
  set(FBXSDK_VERSION "2020.0.1")
endif()

set(_fbxsdk_vstudio_version "vs2017")

message("Looking for FBX SDK version: ${FBXSDK_VERSION}")

if (NOT DEFINED FBXSDK_SDKS)
   set(FBXSDK_SDKS "$ENV{FBXSDK_ROOT}")
endif()

get_filename_component(FBXSDK_SDKS_ABS ${FBXSDK_SDKS} ABSOLUTE)

set(FBXSDK_WINDOWS_ROOT "${FBXSDK_SDKS_ABS}/${FBXSDK_VERSION}")

set(_fbxsdk_root "${FBXSDK_WINDOWS_ROOT}")
if (ARCH_32)
  set(_fbxsdk_libdir_debug "lib/${_fbxsdk_vstudio_version}/x86/debug")
  set(_fbxsdk_libdir_release "lib/${_fbxsdk_vstudio_version}/x86/release")
else()
  set(_fbxsdk_libdir_debug "lib/${_fbxsdk_vstudio_version}/x64/debug")
  set(_fbxsdk_libdir_release "lib/${_fbxsdk_vstudio_version}/x64/release")
endif()
set(_fbxsdk_libname_debug "libfbxsdk.lib")
set(_fbxsdk_libname_release "libfbxsdk.lib")

# should point the the FBX SDK installation dir
set(FBXSDK_ROOT "${_fbxsdk_root}")
message("FBXSDK_ROOT: ${FBXSDK_ROOT}")

# find header dir and libs
find_path(FBXSDK_INCLUDE_DIR "fbxsdk.h"
  NO_CMAKE_FIND_ROOT_PATH
  PATHS ${FBXSDK_ROOT}
  PATH_SUFFIXES "include")
message("FBXSDK_INCLUDE_DIR: ${FBXSDK_INCLUDE_DIR}")

find_library(FBXSDK_LIBRARY ${_fbxsdk_libname_release}
  NO_CMAKE_FIND_ROOT_PATH
  PATHS "${FBXSDK_ROOT}/${_fbxsdk_libdir_release}")
message("FBXSDK_LIBRARY: ${FBXSDK_LIBRARY}")

find_library(FBXSDK_LIBRARY_DEBUG ${_fbxsdk_libname_debug}
  NO_CMAKE_FIND_ROOT_PATH
  PATHS "${FBXSDK_ROOT}/${_fbxsdk_libdir_debug}")
message("FBXSDK_LIBRARY_DEBUG: ${FBXSDK_LIBRARY_DEBUG}")

find_package_handle_standard_args(FbxSdk DEFAULT_MSG
                                  FBXSDK_LIBRARY FBXSDK_INCLUDE_DIR)

mark_as_advanced(FBXSDK_INCLUDE_DIR FBXSDK_LIBRARY)

set(FBXSDK_INCLUDE_DIRS ${FBXSDK_INCLUDE_DIR})
set(FBXSDK_LIBRARIES ${FBXSDK_LIBRARY})
set(FBXSDK_LIBRARIES_DEBUG ${FBXSDK_LIBRARY_DEBUG})