add_library(synchronous_product INTERFACE)
find_package(spdlog REQUIRED)
target_link_libraries(synchronous_product INTERFACE ta mtl utilities spdlog::spdlog)
target_include_directories(synchronous_product INTERFACE include)
