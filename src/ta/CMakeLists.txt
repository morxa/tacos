find_package(range-v3 REQUIRED)
add_library(ta SHARED ta.cpp automata.cpp ata_formula.cpp ata.cpp)
target_link_libraries(ta PRIVATE range-v3::range-v3)
target_include_directories(ta PUBLIC include ${Boost_INCLUDE_DIR})
