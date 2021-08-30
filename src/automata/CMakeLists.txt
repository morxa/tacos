add_library(automata SHARED ta.cpp automata.cpp ata.cpp ta_regions.cpp)
target_link_libraries(automata PUBLIC range-v3::range-v3 utilities fmt::fmt NamedType)
target_include_directories(
  automata
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tacos>
         ${Boost_INCLUDE_DIR})

install(
  TARGETS automata
  EXPORT TacosTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

install(DIRECTORY include/automata
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos)

find_package(Protobuf QUIET)
if(Protobuf_FOUND)
  message(STATUS "Protobuf found, building TA proto library")
  protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ta.proto)
  add_library(ta_proto SHARED ta_proto.cpp ${PROTO_SRCS} ${PROTO_HDRS})
  target_link_libraries(
    ta_proto
    PUBLIC automata protobuf::libprotobuf
    PRIVATE range-v3::range-v3)
  target_include_directories(
    ta_proto
    PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
           $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>
           $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tacos>)
  install(
    TARGETS ta_proto
    EXPORT TacosTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(FILES ta.proto DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/tacos)
  install(FILES ${PROTO_HDRS}
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos/automata)
else()
  message(STATUS "Protobuf not found, skipping TA proto library")
endif()
