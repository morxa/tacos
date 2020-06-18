find_package(Catch2 REQUIRED)

include(Catch)

add_executable(testta test_ta.cpp catch2_main.cpp)
target_link_libraries(testta PRIVATE libta PRIVATE Catch2::Catch2)

add_executable(testinterval test_interval.cpp catch2_main.cpp)
target_link_libraries(testinterval PRIVATE libmtl PRIVATE Catch2::Catch2)

add_executable(testmtlformulae test_mtlFormula.cpp catch2_main.cpp)
target_link_libraries(testmtlformulae PRIVATE libmtl PRIVATE Catch2::Catch2)

catch_discover_tests(testta)
catch_discover_tests(testinterval)
catch_discover_tests(testmtlformulae)
