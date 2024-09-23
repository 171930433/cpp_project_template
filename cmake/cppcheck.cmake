# sudo apt-get install cppcheck
find_program(CPPCHECK cppcheck)

add_custom_target(cppcheck
    COMMAND ${CPPCHECK} --enable=all ${CMAKE_SOURCE_DIR} -i ${CMAKE_BINARY_DIR} > ${CMAKE_BINARY_DIR}/cppcheck_result.txt
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Running Cppcheck"
)