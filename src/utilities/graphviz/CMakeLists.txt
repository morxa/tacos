find_package(PkgConfig QUIET REQUIRED)
pkg_check_modules(gvc libgvc QUIET IMPORTED_TARGET)

if(gvc_FOUND)
  message(STATUS "Found graphviz, building wrapper library")
  add_library(graphviz SHARED graph.cpp node.cpp)
  target_include_directories(
    graphviz PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                    $<INSTALL_INTERFACE:tacos>)
  target_link_libraries(graphviz PUBLIC gvc PkgConfig::gvc)
  install(
    TARGETS graphviz
    EXPORT TacosTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(DIRECTORY include/utilities/graphviz
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos/utilities)
else()
  message(STATUS "Could not find graphviz, omitting graphviz wrapper library")
endif()
