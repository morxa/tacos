add_library(golog_adapter SHARED golog_program.cpp golog_adapter.cpp golog_symbols.cpp)
target_link_libraries(golog_adapter PUBLIC search readylog++ parsegolog++ utilities)
target_include_directories(
  golog_adapter PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tacos>)

install(
  TARGETS golog_adapter
  EXPORT TacosTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/golog_adapter DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos)
