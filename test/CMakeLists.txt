find_package(Catch2 REQUIRED)

include(Catch)

add_executable(testta test_ta.cpp catch2_main.cpp)
target_link_libraries(testta PRIVATE libta PRIVATE Catch2::Catch2)
catch_discover_tests(testta)
