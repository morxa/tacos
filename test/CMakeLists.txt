find_package(Catch2 REQUIRED)

include(Catch)

add_executable(testta test_ta.cpp test_ta_region.cpp catch2_main.cpp)
target_link_libraries(testta PRIVATE ta PRIVATE Catch2::Catch2)
catch_discover_tests(testta)

add_executable(testinterval test_interval.cpp catch2_main.cpp)
target_link_libraries(testinterval PRIVATE mtl PRIVATE Catch2::Catch2 PRIVATE utilities)
catch_discover_tests(testinterval)

add_executable(testnumberutilities test_number_utilities.cpp catch2_main.cpp)
target_link_libraries(testnumberutilities PRIVATE utilities PRIVATE Catch2::Catch2)
catch_discover_tests(testnumberutilities)

add_executable(testmtlformulae test_mtlFormula.cpp catch2_main.cpp)
target_link_libraries(testmtlformulae PRIVATE mtl PRIVATE Catch2::Catch2)
catch_discover_tests(testmtlformulae)

add_executable(testata catch2_main.cpp test_ata_formula.cpp test_ata.cpp test_print_ata.cpp test_print_ata_formula.cpp)
target_link_libraries(testata PRIVATE ta PRIVATE Catch2::Catch2)
catch_discover_tests(testata)

add_executable(test_mtl_ata_translation test_mtl_ata_translation.cpp catch2_main.cpp)
target_link_libraries(test_mtl_ata_translation PRIVATE mtl_ata_translation PRIVATE Catch2::Catch2)
catch_discover_tests(test_mtl_ata_translation)
