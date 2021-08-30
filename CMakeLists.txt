cmake_minimum_required (VERSION 3.14)

# projectname is the same as the main-executable
project(tacos VERSION 0.9.0
  DESCRIPTION "Timed Automata Controller Synthesis")

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

include(GNUInstallDirs)
include(CTest)
option(MTLSYN_BUILD_DOC "Allow building documentation, requires doxygen." ON)
option(COVERAGE "Generate coverage report" OFF)
option(BUILD_BENCHMARKS "Build benchmarks" OFF)

add_compile_options(-W -Wall -Werror -Wextra -Wpedantic)
set(CMAKE_CXX_STANDARD 17)

include(third_party/third_party.cmake)

if (COVERAGE)
	include(CodeCoverage)
	append_coverage_compiler_flags()
endif ()

add_subdirectory(src)

if (BUILD_TESTING)
  add_subdirectory(test)
endif()

if(${MTLSYN_BUILD_DOC})
	find_package(Doxygen REQUIRED dot)
	if (DOXYGEN_FOUND)
  		set(DOXYGEN_OUTPUT_DIRECTORY api)
      set(DOXYGEN_DISTRIBUTE_GROUP_DOC 1)
  		set(DOXYGEN_WARN_AS_ERROR 1)
  		doxygen_add_docs(doc src)
	endif()
endif()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${CMAKE_CURRENT_BINARY_DIR}/TacosConfigVersion.cmake
  VERSION ${PROJECT_VERSION}
  COMPATIBILITY SameMajorVersion)

install(
  EXPORT TacosTargets
  FILE TacosTargets.cmake
  NAMESPACE tacos::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tacos)

install(FILES cmake/TacosConfig.cmake
  ${CMAKE_CURRENT_BINARY_DIR}/TacosConfigVersion.cmake
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/tacos)
