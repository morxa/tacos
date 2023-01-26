add_library(utilities INTERFACE)
target_link_libraries(utilities INTERFACE automata tinyxml2 pthread fmt)
target_include_directories(
  utilities INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                      $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tacos>)

install(TARGETS utilities EXPORT TacosTargets)
install(DIRECTORY include/utilities
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos)
install(DIRECTORY include/io DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos)

add_subdirectory(graphviz)
