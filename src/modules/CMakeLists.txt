set(MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/../)

option(ENABLE_PSINS "enable psins module" ON)

if(ENABLE_PSINS)
  add_subdirectory(psins)
endif()
