# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file LICENSE.rst or https://cmake.org/licensing for details.

#[=======================================================================[.rst:
FindPerl
--------

Find a Perl interpreter.

This module defines the following variables:

``PERL_EXECUTABLE``
  The full path to Perl.

``PERL_FOUND``
  True if the Perl executable was found.

``PERL_VERSION_STRING``
  .. versionadded:: 2.8.8

  The version of Perl found.
#]=======================================================================]

include(${CMAKE_CURRENT_LIST_DIR}/FindCygwin.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/FindMsys.cmake)

set(PERL_POSSIBLE_BIN_PATHS
  ${CYGWIN_INSTALL_PATH}/bin
  ${MSYS_INSTALL_PATH}/usr/bin
  )

if(WIN32)
  get_filename_component(
    ActivePerl_CurrentVersion
    "[HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActivePerl;CurrentVersion]"
    NAME)
  set(PERL_POSSIBLE_BIN_PATHS ${PERL_POSSIBLE_BIN_PATHS}
    "C:/Perl/bin"
    "C:/Strawberry/perl/bin"
    [HKEY_LOCAL_MACHINE\\SOFTWARE\\ActiveState\\ActivePerl\\${ActivePerl_CurrentVersion}]/bin
    )
endif()

find_program(PERL_EXECUTABLE
  NAMES perl
  PATHS ${PERL_POSSIBLE_BIN_PATHS}
  )

if(PERL_EXECUTABLE)
  ### PERL_VERSION
  execute_process(
    COMMAND
      ${PERL_EXECUTABLE} -V:version
      OUTPUT_VARIABLE
        PERL_VERSION_OUTPUT_VARIABLE
      RESULT_VARIABLE
        PERL_VERSION_RESULT_VARIABLE
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
  )
  if(NOT PERL_VERSION_RESULT_VARIABLE AND NOT PERL_VERSION_OUTPUT_VARIABLE MATCHES "^version='UNKNOWN'")
    string(REGEX REPLACE "version='([^']+)'.*" "\\1" PERL_VERSION_STRING ${PERL_VERSION_OUTPUT_VARIABLE})
  else()
    execute_process(
      COMMAND ${PERL_EXECUTABLE} -v
      OUTPUT_VARIABLE PERL_VERSION_OUTPUT_VARIABLE
      RESULT_VARIABLE PERL_VERSION_RESULT_VARIABLE
      ERROR_QUIET
      OUTPUT_STRIP_TRAILING_WHITESPACE
    )
    if(NOT PERL_VERSION_RESULT_VARIABLE AND PERL_VERSION_OUTPUT_VARIABLE MATCHES "This is perl.*[ \\(]v([0-9\\._]+)[ \\)]")
      set(PERL_VERSION_STRING "${CMAKE_MATCH_1}")
    elseif(NOT PERL_VERSION_RESULT_VARIABLE AND PERL_VERSION_OUTPUT_VARIABLE MATCHES "This is perl, version ([0-9\\._]+) +")
      set(PERL_VERSION_STRING "${CMAKE_MATCH_1}")
    endif()
  endif()
endif()

# Deprecated settings for compatibility with CMake1.4
set(PERL ${PERL_EXECUTABLE})

include(FindPackageHandleStandardArgs)
if (CMAKE_FIND_PACKAGE_NAME STREQUAL "PerlLibs")
  # FindPerlLibs include()'s this module. It's an old pattern, but rather than
  # trying to suppress this from outside the module (which is then sensitive to
  # the contents, detect the case in this module and suppress it explicitly.
  set(FPHSA_NAME_MISMATCHED 1)
endif ()
find_package_handle_standard_args(Perl
                                  REQUIRED_VARS PERL_EXECUTABLE
                                  VERSION_VAR PERL_VERSION_STRING)
unset(FPHSA_NAME_MISMATCHED)

mark_as_advanced(PERL_EXECUTABLE)
