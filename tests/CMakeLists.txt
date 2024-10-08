# vcpkg
file(GLOB TEST_FILES *.cpp)

find_package(GTest CONFIG REQUIRED)
# find_package(glog CONFIG REQUIRED) set(GFLAGS_USE_TARGET_NAMESPACE ON)
# find_package(gflags CONFIG REQUIRED)

add_executable(main_test ${TEST_FILES})
target_link_libraries(
  main_test
  PRIVATE ${PROJECT_NAME}
          $<$<BOOL:${FETCH_THIRD_PARTY_DEMO}>:${PROJECT_NAME}_3rd>
  PRIVATE GTest::gtest GTest::gmock)

target_compile_features(main_test PRIVATE cxx_std_20)

include(GoogleTest)

gtest_discover_tests(
  main_test EXTRA_ARGS "--config_dir=${CMAKE_SOURCE_DIR}/config/demo"
                       "--data_dir=${CMAKE_SOURCE_DIR}/data")

option(ENABLE_USE_TARGET_TEST "enable use target tests" OFF)

if(ENABLE_USE_TARGET_TEST)
  add_test(
    NAME use-target_configure
    COMMAND
      ${CMAKE_COMMAND} -H${CMAKE_CURRENT_LIST_DIR}/use_target
      -B${CMAKE_CURRENT_BINARY_DIR}/build_use-target -G${CMAKE_GENERATOR}
      -DCMAKE_PREFIX_PATH=${CMAKE_INSTALL_PREFIX}
      -DCMAKE_BUILD_TYPE=$<CONFIGURATION>)

  set_tests_properties(use-target_configure PROPERTIES FIXTURES_SETUP
                                                       use-target)

  add_test(
    NAME use-target_build
    COMMAND
      ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/build_use-target
      --config $<CONFIGURATION>)

  set_tests_properties(use-target_build PROPERTIES FIXTURES_REQUIRED use-target)

  add_test(
    NAME use-target_test
    COMMAND
      ${CMAKE_COMMAND} --build ${CMAKE_CURRENT_BINARY_DIR}/build_use-target
      --target test --config $<CONFIGURATION>)

  set_tests_properties(use-target_test PROPERTIES FIXTURES_REQUIRED use-target)

  add_test(NAME use-target_cleanup
           COMMAND ${CMAKE_COMMAND} -E remove_directory
                   ${CMAKE_CURRENT_BINARY_DIR}/build_use-target)

  set_tests_properties(use-target_cleanup PROPERTIES FIXTURES_CLEANUP
                                                     use-target)
endif()
