find_package(Boost REQUIRED COMPONENTS program_options)
find_package(Protobuf REQUIRED)
add_library(golog_app SHARED app.cpp)
target_link_libraries(
  golog_app
  PUBLIC automata
  ta_proto
  mtl_ata_translation
  search
  mtl_proto
  visualization
  Boost::program_options
  golog_adapter
  fmt::fmt)
target_include_directories(
  golog_app PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tacos>)
install(
  TARGETS golog_app
  EXPORT GologTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/golog_app DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos)

add_executable(golog_app-exc main.cpp)
target_link_libraries(golog_app-exc PRIVATE golog_app)
install(
  TARGETS golog_app-exc
  EXPORT GologTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
