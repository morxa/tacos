add_library(mtl SHARED MTLFormula.cpp)
target_include_directories(mtl PUBLIC include)
target_link_libraries(mtl PUBLIC utilities)
