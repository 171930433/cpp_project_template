set(MODULES_DIR ${CMAKE_CURRENT_LIST_DIR}/../)

if(ENABLE_PSINS)
  add_subdirectory(psins)
endif()

if(ENABLE_LM)
  add_subdirectory(lm)
endif()
