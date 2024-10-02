file(GLOB_RECURSE HEADER_FILES *.h)
file(GLOB_RECURSE SOURCE_FILES *.cpp)

add_library(psins SHARED ${SOURCE_FILES})

target_include_directories(
  psins
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
  INTERFACE
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/modules/psins>
)

target_compile_definitions(psins PUBLIC PSINS_FOR_LINUX)

target_compile_options(psins PRIVATE -Wno-write-strings -Wno-format -Wno-varargs)

foreach(single_header ${HEADER_FILES})
  file(RELATIVE_PATH relative_header ${MODULES_DIR} ${single_header})
  get_filename_component(header_dir ${relative_header} DIRECTORY)
  install(FILES ${single_header}
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${header_dir})
endforeach()
