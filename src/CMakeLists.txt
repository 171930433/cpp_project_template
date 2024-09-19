
add_library(${PROJECT_NAME} mylib.cpp)

target_include_directories(${PROJECT_NAME}
PUBLIC 
    $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/include>
)

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} 
EXPORT_FILE_NAME ${CMAKE_BINARY_DIR}/include/export.h
)
