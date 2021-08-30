add_library(utilities INTERFACE)
target_link_libraries(utilities INTERFACE ta tinyxml2 pthread)
target_include_directories(
  utilities INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                      $<INSTALL_INTERFACE:tacos>)

install(TARGETS utilities EXPORT TacosTargets)
install(DIRECTORY include/utilities
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos)
install(DIRECTORY include/io DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos)

add_subdirectory(graphviz)
