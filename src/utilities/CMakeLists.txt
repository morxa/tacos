add_library(utilities INTERFACE)
target_link_libraries(utilities INTERFACE pthread)
target_include_directories(utilities INTERFACE include)

add_subdirectory(graphviz)
add_subdirectory(io)
