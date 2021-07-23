add_library(utilities INTERFACE)
target_link_libraries(utilities INTERFACE ta tinyxml2 pthread)
target_include_directories(utilities INTERFACE include)

add_subdirectory(graphviz)
