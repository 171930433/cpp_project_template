# find_package(imgui CONFIG REQUIRED)
# find_package(implot CONFIG REQUIRED)

# library target
add_library(vtkDearImGuiInjector STATIC vtkDearImGuiInjector.cxx ${lib_sources})

target_compile_definitions(vtkDearImGuiInjector PUBLIC ${lib_definitions})
target_include_directories(
  vtkDearImGuiInjector
  PUBLIC ${PORTABLE_FILE_DIALOGS_INCLUDE_DIRS}
         $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}>
         $<BUILD_INTERFACE:${CMAKE_CURRENT_LIST_DIR}>
         $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}>
         $<INSTALL_INTERFACE:include>)
target_link_libraries(vtkDearImGuiInjector PUBLIC ${VTK_LIBRARIES} imgui::imgui
                                                  implot::implot)


# Install targets
install(
  TARGETS vtkDearImGuiInjector
  RUNTIME DESTINATION bin
  LIBRARY DESTINATION lib
  ARCHIVE DESTINATION lib)
install(FILES vtkDearImGuiInjector.h
              ${CMAKE_CURRENT_BINARY_DIR}/vtkDearImGuiInjector_export.h
        DESTINATION include)
install(FILES ${lib_headers} DESTINATION include)
