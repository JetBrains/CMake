add_test(NAME FindProtobuf.Test COMMAND
  ${CMAKE_CTEST_COMMAND} -C $<CONFIGURATION>
  --build-and-test
  "${CMake_SOURCE_DIR}/Tests/FindProtobuf/Test"
  "${CMake_BINARY_DIR}/Tests/FindProtobuf/Test"
  ${build_generator_args}
  --build-project TestFindProtobuf
  --build-options ${build_options}
  "-DCMake_TEST_FindProtobuf_gRPC=${CMake_TEST_FindProtobuf_gRPC}"
  --test-command ${CMAKE_CTEST_COMMAND} -V -C $<CONFIGURATION>
  )
set_property(TEST FindProtobuf.Test PROPERTY FAIL_REGULAR_EXPRESSION PROTOC_EXE)
