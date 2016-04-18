# ============================================================================
# Copyright (c) 2011-2012 University of Pennsylvania
# Copyright (c) 2013-2016 Andreas Schuh
# All rights reserved.
#
# See COPYING file for license information or visit
# https://cmake-basis.github.io/download.html#license
# ============================================================================

##############################################################################
# @file  ConfigureIncludeFiles.cmake
# @brief CMake script used to configure and copy the public header files.
#
# Besides configuring the files, this script optionally copies the header
# files to the build tree using the final relative path as used for the
# installation. This could be done directly during the configure step of
# CMake by code executed as part of the CMakeLists.txt files, but then
# whenever a header file is modified, CMake reconfigures the build system.
# Instead, this script is executed using execute_process() during the
# configure step of CMake and a custom build target is added which rebuilds
# whenever a header file was modified. Thus, only this script is re-executed,
# but not the entire build system re-configured.
#
# The relative path of each configured input header file in the source tree
# is appended to the output log file. This file can be used to determine
# whether a new header was added to the source tree and thus this script has
# to be re-executed.
#
# @ingroup CMakeUtilities
##############################################################################

# ----------------------------------------------------------------------------
# requires bug fixed get_filename_component() of BASIS tools
include ("${CMAKE_CURRENT_LIST_DIR}/CommonTools.cmake")

# ----------------------------------------------------------------------------
# check arguments
if (NOT PROJECT_INCLUDE_DIRS)
  message (FATAL_ERROR "Missing argument PROJECT_INCLUDE_DIRS!")
endif ()

if (NOT BINARY_INCLUDE_DIR)
  message (FATAL_ERROR "Missing argument BINARY_INCLUDE_DIR!")
endif ()

if (NOT EXTENSIONS)
  message (FATAL_ERROR "Missing argument EXTENSIONS!")
endif ()

if (NOT VARIABLE_NAME)
  set (VARIABLE_NAME "PUBLIC_HEADERS")
endif ()

if (NOT COPY_MODE)
  set (COPY_MODE COPYONLY)
endif ()

# ----------------------------------------------------------------------------
# include file which defines CMake variables for use in .h.in files
if (CACHE_FILE)
  include ("${CACHE_FILE}")
endif ()

# ----------------------------------------------------------------------------
# configure header files
foreach (INCLUDE_DIR IN LISTS PROJECT_INCLUDE_DIRS)
  foreach (E IN LISTS EXTENSIONS)
    list (APPEND PATTERN "${INCLUDE_DIR}/*${E}")
  endforeach ()
  file (GLOB_RECURSE FILES RELATIVE "${INCLUDE_DIR}" ${PATTERN})
  foreach (HEADER IN LISTS FILES)
    get_filename_component (SOURCE "${INCLUDE_DIR}/${HEADER}" ABSOLUTE)
    if (NOT PREVIEW AND HEADER MATCHES "\\.in$")
      string (REGEX REPLACE "\\.in$" "" HEADER "${HEADER}")
      configure_file ("${SOURCE}" "${BINARY_INCLUDE_DIR}/${HEADER}" ${COPY_MODE})
    endif ()
    list (APPEND CONFIGURED_HEADERS "${SOURCE}")
  endforeach ()
endforeach ()

# ----------------------------------------------------------------------------
# write CMake script with list of public header files
if (CMAKE_FILE)
  if (_CONFIGURED_HEADERS)
    list (SORT _CONFIGURED_HEADERS) # deterministic order
  endif ()
  file (WRITE "${CMAKE_FILE}" "# Automatically generated by BASIS. DO NOT edit this file!\nset (${VARIABLE_NAME}\n")
  foreach (HEADER IN LISTS CONFIGURED_HEADERS)
    file (APPEND "${CMAKE_FILE}" "  \"${HEADER}\"\n")
  endforeach ()
  file (APPEND "${CMAKE_FILE}" ")\n")
endif ()
