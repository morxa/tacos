if(TARGET graphviz)
  add_library(visualization SHARED visualization.cpp)
  target_link_libraries(visualization PUBLIC graphviz search fmt::fmt)
  target_include_directories(
    visualization PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tacos>)
  install(
    TARGETS visualization
    EXPORT TacosTargets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
  install(DIRECTORY include/visualization
          DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos)
endif()
