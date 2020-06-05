find_package(Catch2 REQUIRED)

include(Catch)

add_executable(testta test_ta.cpp catch2_main.cpp)
target_link_libraries(testta PRIVATE libta PRIVATE Catch2::Catch2)
catch_discover_tests(testta)

add_executable(testata catch2_main.cpp test_ata_formula.cpp)
target_link_libraries(testata PRIVATE libta PRIVATE Catch2::Catch2)
catch_discover_tests(testata)
