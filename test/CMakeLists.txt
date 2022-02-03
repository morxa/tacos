if (TACOS_COVERAGE)
  include(CodeCoverage)
endif()

add_library(railroad SHARED railroad.cpp)
target_link_libraries(railroad PUBLIC automata search mtl visualization)

add_library(railroad_location SHARED railroad_location.cpp)
target_link_libraries(railroad_location PUBLIC automata search mtl visualization)

add_library(fischer SHARED fischer.cpp)
target_link_libraries(fischer PUBLIC automata search)

add_library(csma_cd SHARED csma_cd.cpp)
target_link_libraries(csma_cd PUBLIC automata search)

add_executable(test_clock test_clock.cpp)
target_link_libraries(test_clock PRIVATE automata Catch2::Catch2WithMain)
catch_discover_tests(test_clock)

add_executable(testta test_ta.cpp test_ta_region.cpp test_ta_print.cpp test_ta_product.cpp)
target_link_libraries(testta PRIVATE automata PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(testta)

add_executable(testinterval test_interval.cpp)
target_link_libraries(testinterval PRIVATE mtl PRIVATE Catch2::Catch2WithMain PRIVATE utilities)
catch_discover_tests(testinterval)

add_executable(testnumberutilities test_number_utilities.cpp)
target_link_libraries(testnumberutilities PRIVATE utilities PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(testnumberutilities)

add_executable(test_powerset test_powerset.cpp)
target_link_libraries(test_powerset PRIVATE utilities Catch2::Catch2WithMain)
catch_discover_tests(test_powerset)

add_executable(testmtlformulae test_mtlFormula.cpp test_print_mtl_formula.cpp)
target_link_libraries(testmtlformulae PRIVATE mtl PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(testmtlformulae)

add_executable(testata test_ata_formula.cpp test_ata.cpp test_print_ata.cpp test_print_ata_formula.cpp)
target_link_libraries(testata PRIVATE automata PRIVATE Catch2::Catch2WithMain)
catch_discover_tests(testata)

add_executable(test_mtl_ata_translation test_mtl_ata_translation.cpp)
target_link_libraries(test_mtl_ata_translation PRIVATE mtl_ata_translation Catch2::Catch2WithMain spdlog::spdlog)
catch_discover_tests(test_mtl_ata_translation)

add_executable(test_synchronous_product test_synchronous_product.cpp test_synchronous_product_print.cpp)
target_link_libraries(test_synchronous_product PRIVATE mtl_ata_translation search Catch2::Catch2WithMain)
catch_discover_tests(test_synchronous_product)

add_executable(test_create_controller test_create_controller.cpp)
target_link_libraries(test_create_controller PRIVATE railroad mtl_ata_translation search Catch2::Catch2WithMain)
if (TARGET visualization)
  message(STATUS "Building test_create_controller with visualization")
  target_link_libraries(test_create_controller PRIVATE visualization)
  target_compile_options(test_create_controller PRIVATE "-DHAVE_VISUALIZATION")
endif()
catch_discover_tests(test_create_controller)

add_executable(test_search test_search.cpp)
target_link_libraries(test_search PRIVATE mtl_ata_translation search visualization Catch2::Catch2WithMain)
catch_discover_tests(test_search)

add_executable(test_railroad test_railroad.cpp)
target_link_libraries(test_railroad PRIVATE railroad mtl_ata_translation search Catch2::Catch2WithMain)
catch_discover_tests(test_railroad)

add_executable(test_railroad_location test_railroad_location.cpp)
target_link_libraries(test_railroad_location PRIVATE railroad_location mtl_ata_translation search Catch2::Catch2WithMain visualization)
catch_discover_tests(test_railroad_location)

if (TARGET visualization)
  message(STATUS "Building railroad with visualization")
  target_link_libraries(test_railroad PRIVATE visualization)
  target_compile_options(test_railroad PRIVATE "-DHAVE_VISUALIZATION")
endif()

add_executable(test_fischer test_fischer.cpp)
target_link_libraries(test_fischer PRIVATE fischer mtl_ata_translation search Catch2::Catch2WithMain)
catch_discover_tests(test_fischer)
if (TARGET visualization)
  message(STATUS "Building fischer with visualization")
  target_link_libraries(test_fischer PRIVATE visualization)
  target_compile_options(test_fischer PRIVATE "-DHAVE_VISUALIZATION")
endif()

add_executable(test_csma_cd test_csma_cd.cpp)
target_link_libraries(test_csma_cd PRIVATE csma_cd mtl_ata_translation search Catch2::Catch2WithMain)
catch_discover_tests(test_csma_cd)
if (TARGET visualization)
  message(STATUS "Building csma_cd with visualization")
  target_link_libraries(test_csma_cd PRIVATE visualization)
  target_compile_options(test_csma_cd PRIVATE "-DHAVE_VISUALIZATION")
endif()

add_executable(test_priority_thread_pool test_priority_thread_pool.cpp)
target_link_libraries(test_priority_thread_pool PRIVATE utilities Catch2::Catch2WithMain)
catch_discover_tests(test_priority_thread_pool)

add_executable(test_heuristics test_heuristics.cpp)
target_link_libraries(test_heuristics PRIVATE search Catch2::Catch2WithMain)
catch_discover_tests(test_heuristics)

find_package(Protobuf QUIET)

if (Protobuf_FOUND)

  add_executable(test_ta_proto test_ta_proto.cpp)
  target_link_libraries(test_ta_proto PRIVATE automata ta_proto Catch2::Catch2WithMain)
  catch_discover_tests(test_ta_proto)

  add_executable(test_mtl_proto test_mtl_proto.cpp)
  target_link_libraries(test_mtl_proto PRIVATE mtl mtl_proto Catch2::Catch2WithMain)
  catch_discover_tests(test_mtl_proto)

endif()

if (TARGET graphviz)
  add_executable(test_graphviz test_graphviz.cpp)
  target_link_libraries(test_graphviz PRIVATE graphviz Catch2::Catch2WithMain)
  catch_discover_tests(test_graphviz)
endif()

if (TARGET visualization)
    add_executable(test_tree_visualization test_tree_visualization.cpp)
    target_link_libraries(test_tree_visualization PRIVATE visualization Catch2::Catch2WithMain)
    catch_discover_tests(test_tree_visualization)

    add_executable(test_ta_visualization test_ta_visualization.cpp)
    target_link_libraries(test_ta_visualization PRIVATE visualization fmt::fmt Catch2::Catch2WithMain)
    catch_discover_tests(test_ta_visualization)
endif ()

add_executable(test_xml_writer test_xml_writer.cpp)
target_link_libraries(test_xml_writer PRIVATE utilities Catch2::Catch2WithMain)
catch_discover_tests(test_xml_writer)

file(COPY data DESTINATION .)
add_executable(test_app test_app.cpp)
target_link_libraries(test_app PRIVATE app Catch2::Catch2WithMain)
target_compile_options(test_app PRIVATE "-DTEST_DATA_DIR=\"${CMAKE_CURRENT_BINARY_DIR}/data\"")
catch_discover_tests(test_app WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

if (TACOS_GOCOS)
  add_executable(test_golog_words test_golog_words.cpp)
  target_link_libraries(test_golog_words PRIVATE gocos Catch2::Catch2WithMain)
  catch_discover_tests(test_golog_words PROPERTIES ENVIRONMENT READYLOG_PL=$ENV{READYLOG_PL})

  add_executable(test_golog_search test_golog_search.cpp)
  target_link_libraries(test_golog_search PRIVATE gocos search Catch2::Catch2WithMain visualization)
  catch_discover_tests(test_golog_search PROPERTIES ENVIRONMENT READYLOG_PL=$ENV{READYLOG_PL})

  add_executable(test_golog_symbols test_golog_symbols.cpp)
  target_link_libraries(test_golog_symbols PRIVATE gocos NamedType Catch2::Catch2WithMain)
  catch_discover_tests(test_golog_symbols PROPERTIES ENVIRONMENT READYLOG_PL=$ENV{READYLOG_PL})

  add_library(golog_railroad SHARED golog_railroad.cpp)
  target_link_libraries(golog_railroad PUBLIC gocos mtl mtl_ata_translation PRIVATE fmt::fmt)

  add_executable(test_golog_railroad test_golog_railroad.cpp)
  target_link_libraries(test_golog_railroad PRIVATE golog_railroad visualization mtl_ata_translation search Catch2::Catch2WithMain)
  catch_discover_tests(test_golog_railroad PROPERTIES ENVIRONMENT READYLOG_PL=$ENV{READYLOG_PL})

  add_library(golog_robot SHARED golog_robot.cpp)
  target_link_libraries(golog_robot PUBLIC gocos mtl mtl_ata_translation PRIVATE fmt::fmt)
  add_executable(test_golog_robot test_golog_robot.cpp)
  target_link_libraries(test_golog_robot PRIVATE golog_robot visualization mtl_ata_translation search Catch2::Catch2WithMain)
  catch_discover_tests(test_golog_robot PROPERTIES ENVIRONMENT READYLOG_PL=$ENV{READYLOG_PL})
endif()

if (TACOS_COVERAGE)
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

if (TACOS_BUILD_BENCHMARKS)
  add_executable(tacos_benchmark benchmark.cpp benchmark_robot.cpp benchmark_railroad.cpp benchmark_conveyor_belt.cpp)
  target_link_libraries(tacos_benchmark PRIVATE railroad mtl_ata_translation search benchmark::benchmark)
  if (TACOS_BUILD_LARGE_BENCHMARKS)
    target_compile_options(tacos_benchmark PRIVATE "-DBUILD_LARGE_BENCHMARKS")
  endif()
endif()
