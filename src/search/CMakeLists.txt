add_library(search SHARED search_tree.cpp)
target_link_libraries(search PUBLIC automata mtl utilities spdlog::spdlog
                                    fmt::fmt mtl_ata_translation)
target_include_directories(
  search PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tacos>)
install(
  TARGETS search
  EXPORT TacosTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/search DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos)
