find_package(Boost REQUIRED COMPONENTS program_options)
find_package(Protobuf REQUIRED)
add_library(golog-app SHARED app.cpp)
target_link_libraries(
  golog-app
  PUBLIC automata
         ta_proto
         mtl_ata_translation
         search
         mtl_proto
         visualization
         Boost::program_options
         gocos
         fmt::fmt)
target_include_directories(
  golog-app  PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
             $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tacos>)
install(
  TARGETS golog-app
  EXPORT GologTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/app DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos)

add_executable(golog-app-exc main.cpp)
target_link_libraries(golog-app-exc PRIVATE golog-app)
install(
  TARGETS golog-app-exc
  EXPORT GologTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
