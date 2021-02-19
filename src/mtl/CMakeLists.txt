add_library(mtl INTERFACE)
target_include_directories(mtl INTERFACE include)
target_link_libraries(mtl INTERFACE utilities)
