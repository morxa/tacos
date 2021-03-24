find_package(Catch2 3 QUIET)
if (Catch2_FOUND)
  message(STATUS "Found Catch2 on system")
else()
  message(STATUS "Fetching Catch2")
  include(FetchContent)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG 2c269eb6332bc1dd29047851ae1efc3cd4c260d2
    GIT_SHALLOW TRUE
  )
  FetchContent_MakeAvailable(Catch2)
  list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
endif()

include(Catch)

if (COVERAGE)
  include(CodeCoverage)
endif()

add_executable(test_clock test_clock.cpp)
target_link_libraries(test_clock PRIVATE ta Catch2::Catch2WithMain)
catch_discover_tests(test_clock)

add_executable(testta test_ta.cpp test_ta_region.cpp test_ta_print.cpp test_ta_product.cpp)
target_link_libraries(testta PRIVATE ta PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(testta)

add_executable(testinterval test_interval.cpp)
target_link_libraries(testinterval PRIVATE mtl PRIVATE Catch2::Catch2WithMain PRIVATE utilities)
catch_discover_tests(testinterval)

add_executable(testnumberutilities test_number_utilities.cpp)
target_link_libraries(testnumberutilities PRIVATE utilities PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(testnumberutilities)

add_executable(testmtlformulae test_mtlFormula.cpp test_print_mtl_formula.cpp)
target_link_libraries(testmtlformulae PRIVATE mtl PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(testmtlformulae)

add_executable(testata test_ata_formula.cpp test_ata.cpp test_print_ata.cpp test_print_ata_formula.cpp)
target_link_libraries(testata PRIVATE ta PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(testata)

add_executable(test_mtl_ata_translation test_mtl_ata_translation.cpp)
target_link_libraries(test_mtl_ata_translation PRIVATE mtl_ata_translation PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(test_mtl_ata_translation)

add_executable(test_synchronous_product test_synchronous_product.cpp test_synchronous_product_print.cpp)
target_link_libraries(test_synchronous_product PRIVATE mtl_ata_translation synchronous_product Catch2::Catch2WithMain)
catch_discover_tests(test_synchronous_product)

add_executable(test_search test_search.cpp)
target_link_libraries(test_search PRIVATE mtl_ata_translation synchronous_product Catch2::Catch2WithMain)
target_compile_options(test_search PRIVATE "-DCATCH_CONFIG_CONSOLE_WIDTH=200")
catch_discover_tests(test_search)

add_executable(test_railroad test_railroad.cpp)
target_link_libraries(test_railroad PRIVATE mtl_ata_translation synchronous_product Catch2::Catch2WithMain)
target_compile_options(test_railroad PRIVATE "-DCATCH_CONFIG_CONSOLE_WIDTH=200")
catch_discover_tests(test_railroad)

add_executable(test_priority_thread_pool test_priority_thread_pool.cpp)
target_link_libraries(test_priority_thread_pool PRIVATE utilities Catch2::Catch2WithMain)
catch_discover_tests(test_priority_thread_pool)


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
    EXCLUDE "${FETCHCONTENT_BASE_DIR}/*"
  )
endif()
