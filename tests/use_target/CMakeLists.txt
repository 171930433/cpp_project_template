cmake_minimum_required(VERSION 3.5.0)

project(use_demo)

find_package(mylib REQUIRED)

include(CMakePrintHelpers)
cmake_print_properties(
TARGETS
    mylib
PROPERTIES
    # LOCATION
    IMPORTED_LOCATION_DEBUG
    IMPORTED_LOCATION_RELEASE
    IMPORTED_IMPLIB_DEBUG
    IMPORTED_IMPLIB_RELEASE
    INTERFACE_COMPILE_DEFINITIONS
    INTERFACE_COMPILE_OPTIONS
    INTERFACE_INCLUDE_DIRECTORIES
    INTERFACE_LINK_LIBRARIES
)

add_executable(use_demo use_demo.cpp)

target_link_libraries(use_demo PRIVATE mylib::mylib)

enable_testing()
add_test(NAME use_demos COMMAND $<TARGET_FILE:use_demo>)