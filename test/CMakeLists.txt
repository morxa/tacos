find_package(Catch2 REQUIRED)

include(Catch)

add_executable(testta test_ta.cpp catch2_main.cpp)
target_link_libraries(testta PRIVATE libta PRIVATE Catch2::Catch2)
catch_discover_tests(testta)

add_executable(testinterval test_interval.cpp catch2_main.cpp)
target_link_libraries(testinterval PRIVATE libmtl PRIVATE Catch2::Catch2)
catch_discover_tests(testinterval)

add_executable(testmtlformulae test_mtlFormula.cpp catch2_main.cpp)
target_link_libraries(testmtlformulae PRIVATE libmtl PRIVATE Catch2::Catch2)
catch_discover_tests(testmtlformulae)

add_executable(testata catch2_main.cpp test_ata_formula.cpp test_ata.cpp)
target_link_libraries(testata PRIVATE libta PRIVATE Catch2::Catch2)
catch_discover_tests(testata)