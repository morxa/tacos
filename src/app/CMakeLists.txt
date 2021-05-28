find_package(Boost REQUIRED COMPONENTS program_options)
find_package(spdlog REQUIRED)
find_package(Protobuf REQUIRED)
add_library(app SHARED app.cpp)
target_link_libraries(app PUBLIC
  ta ta_proto mtl_ata_translation search mtl_proto visualization
  Boost::program_options fmt::fmt)
target_include_directories(app PUBLIC include)

add_executable(mtlsyn main.cpp)
target_link_libraries(mtlsyn PRIVATE app)

add_executable(ta_visualizer ta_visualizer.cpp)
target_link_libraries(ta_visualizer PRIVATE ta_proto app visualization Boost::program_options spdlog::spdlog)
