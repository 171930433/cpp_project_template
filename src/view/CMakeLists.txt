find_package(imgui CONFIG REQUIRED)
find_package(implot CONFIG REQUIRED)
find_package(VTK CONFIG REQUIRED)
# find_package(glfw3 CONFIG REQUIRED)
# find_package(GLEW REQUIRED)
find_path(PORTABLE_FILE_DIALOGS_INCLUDE_DIRS "portable-file-dialogs.h")

add_executable(view main.cpp viewer.cpp injector/vtkDearImGuiInjector.cxx)

target_link_libraries(view PRIVATE ${PROJECT_NAME} ${VTK_LIBRARIES} imgui::imgui implot::implot)
# message(VTK_LIBRARIES!!!!!!!!!!!!!!!!!!=${VTK_LIBRARIES})
vtk_module_autoinit(TARGETS view MODULES ${VTK_LIBRARIES})

target_include_directories(
  view
  PUBLIC ${PORTABLE_FILE_DIALOGS_INCLUDE_DIRS}
         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>)

target_compile_options(view PRIVATE -Wno-deprecated-declarations -DBOOST_ALLOW_DEPRECATED_HEADERS)

# hello-imgui与vtk的集成没有研究明白
if(0)
  find_package(hello-imgui CONFIG REQUIRED)
  find_package(nlohmann_json CONFIG REQUIRED)
  hello_imgui_add_app(my_app hello_imgui.cpp)
  target_link_libraries(my_app PRIVATE ${PROJECT_NAME} ${VTK_LIBRARIES})
endif()


# 获取主显示器的分辨率
# sudo apt-get install x11-xserver-utils
# execute_process(
#   COMMAND sh -c "xrandr | grep '*' | awk 'NR==1 {print \$1}'"
#   OUTPUT_VARIABLE SCREEN_RESOLUTION
#   OUTPUT_STRIP_TRAILING_WHITESPACE)

# message(STATUS "Primary screen resolution: ${SCREEN_RESOLUTION}")