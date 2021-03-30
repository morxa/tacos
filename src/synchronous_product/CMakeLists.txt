find_package(spdlog REQUIRED)
add_library(synchronous_product SHARED search_tree.cpp)
target_link_libraries(synchronous_product PUBLIC ta mtl utilities spdlog::spdlog)
target_include_directories(synchronous_product PUBLIC include)
