set(MODULE_NAME "lm")

find_package(Eigen3 CONFIG REQUIRED)
find_package(yalantinglibs REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(units CONFIG REQUIRED)
set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED)
find_package(Taskflow CONFIG REQUIRED)

file(GLOB_RECURSE HEADER_FILES *.h)
file(GLOB_RECURSE SOURCE_FILES *.cpp)

add_library(${MODULE_NAME} SHARED ${SOURCE_FILES})

target_include_directories(
  ${MODULE_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
         $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/src>
         $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>
  INTERFACE
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/modules/${MODULE_NAME}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/src>)

target_compile_features(${MODULE_NAME} PUBLIC cxx_std_20)

target_link_libraries(
  ${MODULE_NAME}
  PRIVATE $<$<BOOL:${PROTOBUF_DEMO}>:${PROJECT_NAME}_proto>
          $<$<BOOL:${FETCH_THIRD_PARTY_DEMO}>:${PROJECT_NAME}_3rd> Eigen3::Eigen
          units::units Boost::system Taskflow::Taskflow
  PUBLIC yalantinglibs::yalantinglibs fmt::fmt gflags::gflags)

foreach(single_header ${HEADER_FILES})
  file(RELATIVE_PATH relative_header ${MODULES_DIR} ${single_header})
  get_filename_component(header_dir ${relative_header} DIRECTORY)
  install(FILES ${single_header}
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${header_dir})
endforeach()
