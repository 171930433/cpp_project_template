find_package(protobuf CONFIG REQUIRED)

file(GLOB_RECURSE PROTO_FILES *.proto)

#! 很奇怪,这个变量不能随便命名 PROTO_SRCS PROTO_HDRS
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})
# message(PROTO_FILES============${PROTO_FILES})
# message(PROTO_SRCS============${PROTO_SRCS})
# message(PROTO_HDRS============${PROTO_HDRS})


add_library(${PROJECT_NAME}_proto OBJECT ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(${PROJECT_NAME}_proto PUBLIC protobuf::libprotobuf)

set_target_properties(${PROJECT_NAME}_proto
PROPERTIES
    POSITION_INDEPENDENT_CODE 1
    DEBUG_POSTFIX "_d"
)

install(
FILES
    ${PROTO_HDRS}
DESTINATION
    ${CMAKE_INSTALL_INCLUDEDIR}/${PROJECT_NAME}/proto
)



