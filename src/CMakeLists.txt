file(GLOB_RECURSE HEADER_FILES  ${PROJECT_SOURCE_DIR}/include/*.h)

add_library(${PROJECT_NAME} SHARED mylib.cpp)

target_include_directories(${PROJECT_NAME}
PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
)

set_target_properties(${PROJECT_NAME}
PROPERTIES
    DEBUG_POSTFIX "_d"
    PUBLIC_HEADER "${CMAKE_BINARY_DIR}/include/export.h"
)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} 
EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/include/export.h
)

# install and export
install(
TARGETS
    ${PROJECT_NAME}
EXPORT
    ${PROJECT_NAME}-targets
ARCHIVE
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT lib
RUNTIME
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT bin
LIBRARY
    DESTINATION ${CMAKE_INSTALL_LIBDIR}
    COMPONENT lib
PUBLIC_HEADER
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
    COMPONENT dev
)

set(CMAKE_DESTINATION_DIR share/cmake/${PROJECT_NAME})

install(
EXPORT
    ${PROJECT_NAME}-targets
NAMESPACE
    "${PROJECT_NAME}::"
DESTINATION
    ${CMAKE_DESTINATION_DIR}
COMPONENT
    dev
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion
)

configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
  INSTALL_DESTINATION ${CMAKE_DESTINATION_DIR}
)

install(
FILES
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
    ${HEADER_FILES}
DESTINATION
    ${CMAKE_DESTINATION_DIR}
)

install(
FILES
    ${HEADER_FILES}
DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
)
