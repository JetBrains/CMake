cmake_minimum_required(VERSION 3.15)

project(TestRequiredArtifacts LANGUAGES C)

include(CTest)

if(CMake_TEST_FindPython2)
  find_package(Python2 REQUIRED COMPONENTS Interpreter Development)
  if (NOT Python2_FOUND)
    message (FATAL_ERROR "Failed to find Python 2")
  endif()
  set(USER_LIBRARY "${Python2_LIBRARY_RELEASE}")
  set(USER_INCLUDE_DIR "${Python2_INCLUDE_DIRS}")
else()
  set(USER_LIBRARY "/path/to/invalid${CMAKE_C_LINK_LIBRARY_SUFFIX}")
  set(USER_INCLUDE_DIR "/path/to/invalid/dir")
endif()

# check some combinations for modules search without interpreter
block(SCOPE_FOR VARIABLES)
  find_package(Python3 REQUIRED COMPONENTS Development.SABIModule)
  if (NOT Python3_FOUND)
    message (FATAL_ERROR "Failed to find Python 3")
  endif()
  if (Python3_Development_FOUND)
    message (FATAL_ERROR "Python 3, COMPONENT 'Development' unexpectedly found")
  endif()
  if (Python3_Interpreter_FOUND)
    message (FATAL_ERROR "Python 3, COMPONENT 'Interpreter' unexpectedly found")
  endif()
  if (Python3_Development.Embed_FOUND)
    message (FATAL_ERROR "Python 3, COMPONENT 'Development.Embed' unexpectedly found")
  endif()
  if (Python3_Development.Module_FOUND)
    message (FATAL_ERROR "Python 3, COMPONENT 'Development.Module' unexpectedly found")
  endif()
  if (NOT Python3_Development.SABIModule_FOUND)
    message (FATAL_ERROR "Python 3, COMPONENT 'Development.SABIModule' not found")
  endif()
  unset(_Python3_SABI_LIBRARY_RELEASE CACHE)
endblock()

block(SCOPE_FOR VARIABLES)
  find_package(Python3 REQUIRED COMPONENTS Development.Module Development.SABIModule)
  if (NOT Python3_FOUND)
    message (FATAL_ERROR "Failed to find Python 3")
  endif()
  if (Python3_Development_FOUND)
    message (FATAL_ERROR "Python 3, COMPONENT 'Development' unexpectedly found")
  endif()
  if (Python3_Interpreter_FOUND)
    message (FATAL_ERROR "Python 3, COMPONENT 'Interpreter' unexpectedly found")
  endif()
  if (Python3_Development.Embed_FOUND)
    message (FATAL_ERROR "Python 3, COMPONENT 'Development.Embed' unexpectedly found")
  endif()
  if (NOT Python3_Development.Module_FOUND)
    message (FATAL_ERROR "Python 3, COMPONENT 'Development.Module' not found")
  endif()
  if (NOT Python3_Development.SABIModule_FOUND)
    message (FATAL_ERROR "Python 3, COMPONENT 'Development.SABIModule' not found")
  endif()
  unset(_Python3_LIBRARY_RELEASE CACHE)
  unset(_Python3_SABI_LIBRARY_RELEASE CACHE)
endblock()


set(components Interpreter Development)
if (CMake_TEST_FindPython3_SABIModule AND WIN32)
  list (APPEND components Development.SABIModule)
endif()
find_package(Python3 REQUIRED COMPONENTS ${components})
if (NOT Python3_FOUND)
  message (FATAL_ERROR "Failed to find Python 3")
endif()


add_test(NAME FindPython.RequiredArtifacts.Interpreter.VALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Interpreter.VALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=TRUE -DCHECK_INTERPRETER=ON
                  "-DPython3_EXECUTABLE=${Python3_EXECUTABLE}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
add_test(NAME FindPython.RequiredArtifacts.Interpreter.INVALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Interpreter.INVALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=FALSE -DCHECK_INTERPRETER=ON
                  "-DPython3_EXECUTABLE=${Python3_EXECUTABLE}-bad${CMAKE_EXECUTABLE_SUFFIX}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

add_test(NAME FindPython.RequiredArtifacts.Library.VALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Library.VALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=TRUE -DCHECK_LIBRARY=ON
                  "-DPython3_LIBRARY=${Python3_LIBRARY_RELEASE}"
                  "-DPYTHON_LIBRARY_RELEASE=${Python3_LIBRARY_RELEASE}"
                  "-DPYTHON_RUNTIME_LIBRARY_RELEASE=${Python3_RUNTIME_LIBRARY_RELEASE}"
                  "-DPYTHON_LIBRARY_DEBUG=${Python3_LIBRARY_DEBUG}"
                  "-DPYTHON_RUNTIME_LIBRARY_DEBUG=${Python3_RUNTIME_LIBRARY_DEBUG}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
add_test(NAME FindPython.RequiredArtifacts.Library.INVALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Library.INVALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=FALSE -DCHECK_LIBRARY=ON
                  "-DPython3_LIBRARY=${USER_LIBRARY}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

add_test(NAME FindPython.RequiredArtifacts.Include.VALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Include.VALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=TRUE -DCHECK_INCLUDE=ON
                  "-DPython3_INCLUDE_DIR=${Python3_INCLUDE_DIRS}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
add_test(NAME FindPython.RequiredArtifacts.Include.INVALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Include.INVALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=FALSE -DCHECK_INCLUDE=ON
                  "-DPython3_INCLUDE_DIR=${USER_INCLUDE_DIR}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

add_test(NAME FindPython.RequiredArtifacts.Interpreter-Library.INVALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Interpreter-Library.INVALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=FALSE -DCHECK_INTERPRETER=ON -DCHECK_LIBRARY=ON
                  "-DPython3_EXECUTABLE=${Python3_EXECUTABLE}"
                  "-DPython3_LIBRARY=${USER_LIBRARY}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

add_test(NAME FindPython.RequiredArtifacts.Library-Include.INVALID COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
  "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/Library-Include.INVALID"
  ${build_generator_args}
  --build-project TestRequiredArtifacts.Check
  --build-options -DPYTHON_IS_FOUND=FALSE -DCHECK_LIBRARY=ON -DCHECK_INCLUDE=ON
                  "-DPython3_LIBRARY=${Python3_LIBRARY_RELEASE}"
                  "-DPython3_INCLUDE_DIR=${USER_INCLUDE_DIR}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )

if (CMake_TEST_FindPython3_SABIModule AND WIN32)
  add_test(NAME FindPython.RequiredArtifacts.SABILibrary.VALID COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
    "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/SABILibrary.VALID"
    ${build_generator_args}
    --build-project TestRequiredArtifacts.Check
    --build-options -DPYTHON_IS_FOUND=TRUE -DCHECK_SABI_LIBRARY=ON
                    "-DPython3_SABI_LIBRARY=${Python3_SABI_LIBRARY_RELEASE}"
                    "-DPYTHON_SABI_LIBRARY_RELEASE=${Python3_SABI_LIBRARY_RELEASE}"
                    "-DPYTHON_RUNTIME_SABI_LIBRARY_RELEASE=${Python3_RUNTIME_SABI_LIBRARY_RELEASE}"
                    "-DPYTHON_SABI_LIBRARY_DEBUG=${Python3_SABI_LIBRARY_DEBUG}"
                    "-DPYTHON_RUNTIME_SABI_LIBRARY_DEBUG=${Python3_RUNTIME_SABI_LIBRARY_DEBUG}"
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
  add_test(NAME FindPython.RequiredArtifacts.SABILibrary.INVALID COMMAND
    ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
    --build-and-test
    "${CMake_SOURCE_DIR}/Tests/FindPython/RequiredArtifacts/Check"
    "${CMake_BINARY_DIR}/Tests/FindPython/RequiredArtifacts/SABILibrary.INVALID"
    ${build_generator_args}
    --build-project TestRequiredArtifacts.Check
    --build-options -DPYTHON_IS_FOUND=FALSE -DCHECK_SABI_LIBRARY=ON
                    "-DPython3_SABI_LIBRARY=${USER_LIBRARY}"
    --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
endif()
