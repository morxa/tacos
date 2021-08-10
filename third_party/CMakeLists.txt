include(FetchContent)
FetchContent_Declare(
  NamedType
  GIT_REPOSITORY https://github.com/joboccara/NamedType
)
FetchContent_MakeAvailable(NamedType)

FetchContent_Declare(
        TinyXML2
        GIT_REPOSITORY https://github.com/leethomason/tinyxml2.git
)
set(tinyxml2_BUILD_TESTING OFF)
FetchContent_MakeAvailable(TinyXML2)

find_package(spdlog QUIET)
if (spdlog_FOUND)
  message(STATUS "Found spdlog on system")
else()
  message(STATUS "Fetching spdlog")
  include(FetchContent)
  FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_SHALLOW TRUE
    GIT_TAG v1.x
  )
  set(SPDLOG_BUILD_SHARED ON)
  FetchContent_MakeAvailable(spdlog)
endif()

find_package(fmt QUIET)
if (fmt_FOUND)
  message(STATUS "Found fmt on system")
else()
  message(STATUS "Fetching fmt")
  include(FetchContent)
  FetchContent_Declare(
   fmt
   GIT_REPOSITORY https://github.com/fmtlib/fmt.git
   GIT_SHALLOW TRUE
  )
  set(BUILD_SHARED_LIBS ON)
  FetchContent_MakeAvailable(fmt)
endif()
