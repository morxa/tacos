find_package(range-v3 REQUIRED)
add_library(libta SHARED ta.cpp automata.cpp ata_formula.cpp)
target_link_libraries(libta PRIVATE range-v3::range-v3)
target_include_directories(libta PUBLIC include)
