include(FetchContent)
FetchContent_Declare(NamedType
                     GIT_REPOSITORY https://github.com/joboccara/NamedType)
FetchContent_MakeAvailable(NamedType)

find_package(tinyxml2 QUIET)
if(tinyxml2_FOUND)
  message(STATUS "Found tinyxml2 on system")
else()
  FetchContent_Declare(
    TinyXML2 GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git)
  set(tinyxml2_BUILD_TESTING OFF)
  FetchContent_MakeAvailable(TinyXML2)
endif()

find_package(fmt 9 QUIET)
set(FORCE_FETCH_SPDLOG OFF)
if(fmt_FOUND)
  message(STATUS "Found fmt on system")
else()
  message(STATUS "Fetching fmt")
  FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_SHALLOW TRUE
    GIT_TAG 9.1.0)
  set(BUILD_SHARED_LIBS ON)
  set(FMT_INSTALL ON)
  # Only use the system version if fmt is also found, otherwise spdlog may use
  # an incompatible version of fmt.
  set(FORCE_FETCH_SPDLOG ON)
  FetchContent_MakeAvailable(fmt)
endif()

if (NOT FORCE_FETCH_SPDLOG)
  find_package(spdlog QUIET)
endif()
if(spdlog_FOUND)
  message(STATUS "Found spdlog on system")
else()
  message(STATUS "Fetching spdlog")
  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_SHALLOW TRUE
    GIT_TAG v1.x)
  set(SPDLOG_INSTALL ON)
  set(SPDLOG_BUILD_SHARED ON)
  set(SPDLOG_FMT_EXTERNAL ON)
  FetchContent_MakeAvailable(spdlog)
endif()

find_package(range-v3 QUIET)
if(range-v3_FOUND)
  message(STATUS "Found range-v3 on system")
else()
  message(STATUS "Fetching range-v3")
  FetchContent_Declare(
    range-v3
    GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(range-v3)
endif()

find_package(Catch2 3 QUIET)
if(Catch2_FOUND)
  message(STATUS "Found Catch2 on system")
else()
  message(STATUS "Fetching Catch2")
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2.git
    GIT_TAG v3.3.0
    GIT_SHALLOW TRUE)
  FetchContent_MakeAvailable(Catch2)
  list(APPEND CMAKE_MODULE_PATH ${Catch2_SOURCE_DIR}/extras)
endif()
include(Catch)

if(TACOS_GOCOS)
  find_package(golog++ 0.2 QUIET)
  if(golog++_FOUND)
    message(STATUS "Found golog++ on system")
  else()
    message(STATUS "Fetching golog++")
    FetchContent_Declare(
      golog++
      GIT_REPOSITORY https://github.com/morxa/gologpp.git
      GIT_TAG master
      GIT_SHALLOW TRUE)
    set(GOLOGPP_BUILD_TESTING OFF)
    FetchContent_MakeAvailable(golog++)
  endif()
endif()

if (TACOS_BUILD_BENCHMARKS)
  FetchContent_Declare(
    googlebenchmark
    GIT_REPOSITORY https://github.com/google/benchmark
    GIT_TAG v1.6.0
  )
  set(BENCHMARK_ENABLE_TESTING OFF)
  FetchContent_MakeAvailable(googlebenchmark)
endif()
