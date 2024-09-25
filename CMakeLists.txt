cmake_minimum_required(VERSION 3.5.0)
project(mylib VERSION 1.0.0 LANGUAGES CXX)

option(FETCH_THIRD_PARTY_DEMO "demo of use protobuf" ON)
if(FETCH_THIRD_PARTY_DEMO)
    include(cmake/third_party.cmake)
endif()


if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
endif()

include(GNUInstallDirs)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_LIBDIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/${CMAKE_INSTALL_BINDIR})
set(CMAKE_DESTINATION_DIR share/cmake/${PROJECT_NAME})

# message(CMAKE_INSTALL_BINDIR=======${CMAKE_INSTALL_BINDIR})

option(ENABLE_CPPCHECK "demo of use static check, cppcheck" ON)
if(ENABLE_CPPCHECK)
    include(cmake/cppcheck.cmake)
endif()

option(PROTOBUF_DEMO "demo of use protobuf" ON)
if(PROTOBUF_DEMO)
    add_subdirectory(proto)
endif()

add_subdirectory(src)

option(ENABLE_TESTS "ENABLE_TESTS OPTIONS" ON)
if(ENABLE_TESTS)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()

option(ENABLE_CPACK "ENABLE_cpack OPTIONS" ON)
if(ENABLE_CPACK)
    set(CPACK_PROJECT_NAME ${PROJECT_NAME})
    set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
    include(CPack)
endif()


