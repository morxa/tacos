add_library(gocos SHARED golog_program.cpp golog_adapter.cpp)
target_link_libraries(gocos PUBLIC search readylog++ parsegolog++ utilities)
target_include_directories(
  gocos PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
               $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tacos>)

install(
  TARGETS gocos
  EXPORT TacosTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/gocos DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos)
