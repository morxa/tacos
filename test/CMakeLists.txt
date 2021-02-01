find_package(Catch2 REQUIRED)

include(Catch)

if (COVERAGE)
  include(CodeCoverage)
endif()

add_executable(test_clock test_clock.cpp catch2_main.cpp)
target_link_libraries(test_clock PRIVATE ta Catch2::Catch2)
catch_discover_tests(test_clock)

add_executable(testta test_ta.cpp test_ta_region.cpp test_ta_print.cpp catch2_main.cpp)
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

add_executable(test_synchronous_product test_synchronous_product.cpp test_synchronous_product_print.cpp catch2_main.cpp)
target_link_libraries(test_synchronous_product PRIVATE mtl_ata_translation synchronous_product Catch2::Catch2)
catch_discover_tests(test_synchronous_product)

add_executable(test_search test_search.cpp catch2_main.cpp)
target_link_libraries(test_search PRIVATE mtl_ata_translation synchronous_product Catch2::Catch2)
catch_discover_tests(test_search)

if (COVERAGE)
  # Depend on all targets in the current directory.
  get_property(test_targets DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    PROPERTY BUILDSYSTEM_TARGETS)
  setup_target_for_coverage_lcov(
    NAME coverage
    EXECUTABLE ctest
    DEPENDENCIES ${test_targets}
    EXCLUDE "/usr/*"
    EXCLUDE "test/*"
  )
endif()
