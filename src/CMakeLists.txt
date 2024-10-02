file(GLOB HEADER_FILES *.h)
file(GLOB SOURCE_FILES mylib.cpp)

option(ENABLE_PSINS "enable psins module" ON)
option(ENABLE_IMGUI "enable IMGUI module" ON)
if(ENABLE_IMGUI)
  find_package(imgui CONFIG REQUIRED)
  find_package(implot CONFIG REQUIRED)
  find_package(hello-imgui CONFIG REQUIRED)
  find_package(nlohmann_json CONFIG REQUIRED)
endif()

# vcpkg
find_package(Eigen3 CONFIG REQUIRED)
find_package(yalantinglibs REQUIRED)
# find_package(glog CONFIG REQUIRED)
find_package(fmt CONFIG REQUIRED)
find_package(units CONFIG REQUIRED)
set(GFLAGS_USE_TARGET_NAMESPACE ON)
find_package(gflags CONFIG REQUIRED)
find_package(Taskflow CONFIG REQUIRED)

# system
find_package(Boost REQUIRED COMPONENTS system)

add_subdirectory(modules)

add_library(${PROJECT_NAME} SHARED ${SOURCE_FILES})

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE $<$<BOOL:${PROTOBUF_DEMO}>:${PROJECT_NAME}_proto>
          $<$<BOOL:${FETCH_THIRD_PARTY_DEMO}>:${PROJECT_NAME}_3rd> Eigen3::Eigen
          units::units Boost::system Taskflow::Taskflow
  PUBLIC yalantinglibs::yalantinglibs fmt::fmt gflags::gflags
         $<$<BOOL:${ENABLE_PSINS}>:implot::implot>
         $<$<BOOL:${ENABLE_PSINS}>:imgui::imgui>
  # modules
  PUBLIC $<$<BOOL:${ENABLE_PSINS}>:psins>)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_20)

target_compile_definitions(
  ${PROJECT_NAME}
  PUBLIC $<$<BOOL:${PROTOBUF_DEMO}>:HAVE_PROTOBUF>
         $<$<BOOL:${FETCH_THIRD_PARTY_DEMO}>:HAVE_THIRD_PARTY>
         $<$<BOOL:${ENABLE_PSINS}>:ENABLE_PSINS>)

target_include_directories(
  ${PROJECT_NAME}
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
  INTERFACE $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}>)

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES DEBUG_POSTFIX "_d" PUBLIC_HEADER
                                "${CMAKE_CURRENT_BINARY_DIR}/export.h")

include(GenerateExportHeader)
generate_export_header(${PROJECT_NAME} EXPORT_FILE_NAME
                       ${CMAKE_CURRENT_BINARY_DIR}/export.h)

# install and export
install(
  TARGETS ${PROJECT_NAME} psins
  EXPORT ${PROJECT_NAME}-targets
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT bin
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR} COMPONENT lib
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}
                COMPONENT dev)

install(
  EXPORT ${PROJECT_NAME}-targets
  NAMESPACE "${PROJECT_NAME}::"
  DESTINATION ${CMAKE_DESTINATION_DIR}
  COMPONENT dev)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

configure_package_config_file(
  ${PROJECT_SOURCE_DIR}/cmake/config.cmake.in
  ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
  INSTALL_DESTINATION ${CMAKE_DESTINATION_DIR})

install(FILES ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}-config-version.cmake
        DESTINATION ${CMAKE_DESTINATION_DIR})

foreach(single_header ${HEADER_FILES})
  file(RELATIVE_PATH relative_header ${CMAKE_CURRENT_LIST_DIR} ${single_header})
  get_filename_component(header_dir ${relative_header} DIRECTORY)
  install(FILES ${single_header}
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/${header_dir})
endforeach()

# my app
if(ENABLE_IMGUI)
  hello_imgui_add_app(my_app my_app.cpp)
  target_link_libraries(my_app PRIVATE ${PROJECT_NAME})
endif()
