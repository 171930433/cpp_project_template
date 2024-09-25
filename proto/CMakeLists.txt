find_package(protobuf CONFIG REQUIRED)

file(GLOB_RECURSE PROTO_FILES *.proto)

# ! 注意,这个变量不能是用 HDRS和SRCS ，即不能和PROTOBUF_GENERATE_CPP函数定义的两个输出参数名称一致
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
# message(PROTO_FILES============${PROTO_FILES})
# message(PROTO_SRCS============${PROTO_SRCS})
# message(PROTO_HDRS============${PROTO_HDRS})

add_library(${PROJECT_NAME}_proto OBJECT ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(${PROJECT_NAME}_proto PUBLIC protobuf::libprotobuf)

set_target_properties(${PROJECT_NAME}_proto PROPERTIES POSITION_INDEPENDENT_CODE
                                                       1 DEBUG_POSTFIX "_d")

install(FILES ${PROTO_HDRS}
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/proto)
