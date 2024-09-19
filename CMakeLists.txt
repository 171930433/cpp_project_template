cmake_minimum_required(VERSION 3.5.0)
project(my_lib VERSION 0.1.0 LANGUAGES C CXX)


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


