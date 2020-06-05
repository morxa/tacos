add_library(libta SHARED ta.cpp automata.cpp ata_formula.cpp)
target_include_directories(libta PUBLIC include)
