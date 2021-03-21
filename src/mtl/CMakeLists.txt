add_library(mtl INTERFACE)
target_include_directories(mtl INTERFACE include)
target_link_libraries(mtl INTERFACE utilities)

find_package(Protobuf REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS mtl.proto)

add_library(mtl_proto SHARED mtl_proto.cpp ${PROTO_SRCS} ${PROTO_HDRS})
target_link_libraries(mtl_proto PUBLIC mtl ${Protobuf_LIBRARIES})
target_include_directories(mtl_proto PUBLIC include ${Protobuf_INCLUDE_DIRS} ${CMAKE_BINARY_DIR}/src)
