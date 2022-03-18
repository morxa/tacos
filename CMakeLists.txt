cmake_minimum_required (VERSION 3.14)

# projectname is the same as the main-executable
project(tacos VERSION 1.0.0
  DESCRIPTION "Timed Automata Controller Synthesis")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(GNUInstallDirs)
include(CTest)
option(TACOS_BUILD_DOC "Allow building documentation, requires doxygen." ON)
option(TACOS_COVERAGE "Generate coverage report" OFF)
option(TACOS_BUILD_BENCHMARKS "Build benchmarks" OFF)
option(TACOS_GOCOS "Build Golog controller synthesis" OFF)
option(TACOS_CLANG_TIDY "Run clang-tidy during build" OFF)

set(CMAKE_CXX_STANDARD 17)

include(third_party/third_party.cmake)

add_compile_options(-W -Wall -Werror -Wextra -Wpedantic)

if (TACOS_CLANG_TIDY)
  set(CLANG_TIDY_CHECKS "-checks=-*,bugprone-*")
  set(CMAKE_CXX_CLANG_TIDY clang-tidy ${CLANG_TIDY_CHECKS}
    -header-filter=${CMAKE_SOURCE_DIR}/src|${CMAKE_SOURCE_DIR}/test
    -warnings-as-errors=*)
endif()

if (TACOS_COVERAGE)
	include(CodeCoverage)
	append_coverage_compiler_flags()
endif ()

add_subdirectory(src)

if (BUILD_TESTING)
  add_subdirectory(test)
endif()

if(TACOS_BUILD_DOC)
  find_package(Doxygen REQUIRED dot)
  set(DOXYGEN_OUTPUT_DIRECTORY api)
  set(DOXYGEN_DISTRIBUTE_GROUP_DOC 1)
  set(DOXYGEN_WARN_AS_ERROR 1)
  set(DOXYGEN_USE_MDFILE_AS_MAINPAGE README.md)
  doxygen_add_docs(doc src README.md)
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/tacos-config-version.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(
  EXPORT TacosTargets
  FILE tacos-targets.cmake
  NAMESPACE tacos::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tacos)

install(FILES cmake/tacos-config.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/tacos-config-version.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tacos)
