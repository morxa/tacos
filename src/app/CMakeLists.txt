find_package(Boost REQUIRED COMPONENTS program_options)
find_package(Protobuf REQUIRED)
add_library(app SHARED app.cpp)
target_link_libraries(
  app
  PUBLIC ta
         ta_proto
         mtl_ata_translation
         search
         mtl_proto
         visualization
         Boost::program_options
         fmt::fmt)
target_include_directories(
  app PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
             $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}/tacos>)
install(
  TARGETS app
  EXPORT TacosTargets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(DIRECTORY include/app DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/tacos)

add_executable(tacos main.cpp)
target_link_libraries(tacos PRIVATE app)
install(
  TARGETS tacos
  EXPORT TacosTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

add_executable(ta_visualizer ta_visualizer.cpp)
target_link_libraries(
  ta_visualizer PRIVATE ta_proto app visualization Boost::program_options
                        spdlog::spdlog)

install(
  TARGETS ta_visualizer
  EXPORT TacosTargets
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
