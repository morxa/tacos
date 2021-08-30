add_library(mtl_ata_translation SHARED translator.cpp)
target_link_libraries(mtl_ata_translation PUBLIC automata mtl fmt::fmt)
target_include_directories(
  mtl_ata_translation
  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
         $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tacos>)
install(
  TARGETS mtl_ata_translation
  EXPORT TacosTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/mtl_ata_translation
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos)
