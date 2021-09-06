find_package(Protobuf QUIET)

add_library(mtl INTERFACE)
target_include_directories(
  mtl INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tacos>)
target_link_libraries(mtl INTERFACE utilities fmt)

install(
  TARGETS mtl
  EXPORT TacosTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/mtl DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos)

if(Protobuf_FOUND)
  message(STATUS "Protobuf found, building MTL proto library")
  protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS mtl.proto)
  add_library(mtl_proto SHARED mtl_proto.cpp ${PROTO_SRCS} ${PROTO_HDRS})
  target_link_libraries(mtl_proto PUBLIC mtl protobuf::libprotobuf fmt::fmt)
  target_include_directories(
    mtl_proto
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tacos>)
  install(
    TARGETS mtl_proto
    EXPORT TacosTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(FILES mtl.proto DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/tacos)
  install(FILES ${PROTO_HDRS} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos/mtl)
else()
  message(STATUS "Protobuf not found, skipping MTL proto library")
endif()
