cmake_minimum_required(VERSION 3.10)

if(POLICY CMP0129)
  cmake_policy(SET CMP0129 NEW)
endif()

project(target_link_libraries)

file(WRITE
  "${CMAKE_CURRENT_BINARY_DIR}/main.cxx"
  "int main() { return 0; }
"
)

add_executable(
  target_link_libraries
  "${CMAKE_CURRENT_BINARY_DIR}/main.cxx"
)

macro(ASSERT_PROPERTY _target _property _value)
  get_target_property(_out ${_target} ${_property})
  if (NOT _out)
    set(_out "")
  endif()
  if (NOT "${_out}" STREQUAL "${_value}")
    message(SEND_ERROR "Target ${_target} does not have property ${_property} with value ${_value}. Actual value: ${_out}")
  endif()
endmacro()

include(GenerateExportHeader)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

add_library(depA SHARED depA.cpp)
generate_export_header(depA)

add_library(depB SHARED depB.cpp)
generate_export_header(depB)

target_link_libraries(depB LINK_PRIVATE depA LINK_PRIVATE depA)

add_library(libgenex SHARED libgenex.cpp)
generate_export_header(libgenex)

set_property(TARGET depB APPEND PROPERTY
  LINK_LIBRARIES $<1:libgenex>
)

add_library(depC SHARED depC.cpp)
generate_export_header(depC)

target_link_libraries(depC LINK_PUBLIC depA LINK_PUBLIC depA)

assert_property(depA INTERFACE_LINK_LIBRARIES "")
assert_property(depB INTERFACE_LINK_LIBRARIES "")
assert_property(depC INTERFACE_LINK_LIBRARIES "depA;depA")

add_executable(targetA targetA.cpp)

target_link_libraries(targetA LINK_INTERFACE_LIBRARIES depA depB)

assert_property(targetA INTERFACE_LINK_LIBRARIES "depA;depB")

set_target_properties(targetA PROPERTIES INTERFACE_LINK_LIBRARIES "")

assert_property(targetA INTERFACE_LINK_LIBRARIES "")

add_subdirectory(subdir)
target_link_libraries(targetA subdirlib)

target_link_libraries(targetA depB depC)

assert_property(targetA INTERFACE_LINK_LIBRARIES ";subdirlib;depB;depC")

# Exclude depIfaceOnly from ALL so that it will only be built if something
# depends on it. As it is in the link interface of depB, targetA
# will depend on it. That dependency is what is being tested here.
add_library(depIfaceOnly SHARED EXCLUDE_FROM_ALL depIfaceOnly.cpp)
generate_export_header(depIfaceOnly)
set_property(TARGET depB APPEND PROPERTY INTERFACE_LINK_LIBRARIES depIfaceOnly)

add_library(depD SHARED depD.cpp)
generate_export_header(depD)
set_property(TARGET depD APPEND PROPERTY
  INTERFACE_LINK_LIBRARIES
    $<$<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>:depA>
)

add_executable(targetB targetB.cpp)
target_link_libraries(targetB depD)

macro(create_header _name)
  file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/${_name}")
  file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/${_name}/${_name}.h" "//${_name}.h\n")
endmacro()

create_header(foo)
create_header(bar)

add_library(depG SHARED depG.cpp)
generate_export_header(depG)
target_include_directories(depG INTERFACE
    "${CMAKE_CURRENT_BINARY_DIR}/foo"
    "${CMAKE_CURRENT_BINARY_DIR}/bar"
)
target_compile_definitions(depG INTERFACE
    TEST_DEF
)


add_executable(targetC targetC.cpp)
if(NOT BORLAND AND NOT WATCOM)
  # Linking to a target containing a + should be non-fatal, though it does
  # not work at all on Borland or watcom
  add_library(wrapc++ empty.cpp)
  target_link_libraries(targetC wrapc++)
endif()
# The TARGET_PROPERTY expression is duplicated below to test that there is no
# shortcutting of the evaluation by returning an empty string.
set(_exe_test $<STREQUAL:$<TARGET_PROPERTY:TYPE>,EXECUTABLE>)
target_link_libraries(targetC $<$<AND:${_exe_test},${_exe_test}>:depG>)

add_library(libConsumer empty.cpp)
# This line causes $<$<CONFIG:Debug>:depA> to be used when
# determining the include directories for libConsumer based on the
# interface properties of its LINK_LIBRARIES. Because the above expression
# evaluates to the empty string in non-Debug cases, ensure that that causes
# no problems.
target_link_libraries(libConsumer debug depA)

add_subdirectory(cmp0022)

add_executable(newsignature1 newsignature1.cpp)
target_link_libraries(newsignature1 PRIVATE depC INTERFACE depD PUBLIC depB PRIVATE subdirlib INTERFACE INTERFACE PUBLIC)

assert_property(newsignature1 INTERFACE_LINK_LIBRARIES "depD;depB")
assert_property(newsignature1 LINK_LIBRARIES "depC;depB;subdirlib")

#----------------------------------------------------------------------------
# Test cross-directory linking.
cmake_policy(PUSH)
cmake_policy(SET CMP0022 NEW)
cmake_policy(SET CMP0079 NEW)
add_executable(TopDir TopDir.c)
add_library(TopDirInterface INTERFACE)
target_link_libraries(TopDir PRIVATE TopDirInterface)
add_subdirectory(SubDirA)
add_subdirectory(SubDirB)
target_link_libraries(SubDirB TopDirImported)
add_subdirectory(SubDirC)
target_link_libraries(SubDirC PRIVATE SubDirC2)
target_link_libraries(TopDir PRIVATE SubDirC)
add_library(TopDirImported IMPORTED INTERFACE)
target_compile_definitions(TopDirImported INTERFACE DEF_TopDirImported)
cmake_policy(POP)

#----------------------------------------------------------------------------
# Test $<COMPILE_ONLY:> genex.
cmake_policy(SET CMP0099 NEW)
add_library(dont_link_too SHARED compile_only.cpp)
target_compile_definitions(dont_link_too PUBLIC USE_EXAMPLE PRIVATE HAVE_FUNCTION)
target_link_options(dont_link_too INTERFACE invalid_link_option)
target_link_libraries(dont_link_too INTERFACE invalid_link_library)

add_library(uses_compile_only_genex SHARED compile_only.cpp)
target_link_libraries(uses_compile_only_genex PUBLIC $<COMPILE_ONLY:dont_link_too>)

add_library(uses_compile_only_genex_static STATIC compile_only.cpp)
target_link_libraries(uses_compile_only_genex_static PRIVATE $<COMPILE_ONLY:dont_link_too>)
add_executable(uses_via_static_linking main.cxx)
target_link_libraries(uses_via_static_linking PRIVATE uses_compile_only_genex_static)

add_library(uses_compile_only_genex_obj SHARED compile_only.cpp)
target_compile_definitions(uses_compile_only_genex_obj PRIVATE HAVE_FUNCTION)
target_link_libraries(uses_compile_only_genex_obj PRIVATE $<COMPILE_ONLY:dont_link_too>)

cmake_policy(SET CMP0131 NEW)
add_library(only_link_too OBJECT link_only.cpp)
target_compile_options(only_link_too INTERFACE INVALID_COMPILER_FLAG_ARRRRGH)

add_executable(uses_link_only_genex_obj compile_only.cpp)
target_compile_definitions(uses_link_only_genex_obj PUBLIC USE_EXAMPLE)
target_link_libraries(uses_link_only_genex_obj PRIVATE $<LINK_ONLY:only_link_too>)
