cmake_minimum_required (VERSION 3.14)

# projectname is the same as the main-executable
project(mtlsyn)

include(GNUInstallDirs)
include(CTest)
option(MTLSYN_BUILD_DOC "Allow building documentation, requires doxygen." ON)
option(COVERAGE "Generate coverage report" OFF)

add_compile_options(-W -Wall -Werror -Wextra -Wpedantic)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if (COVERAGE)
  include(CodeCoverage)
  append_coverage_compiler_flags()
endif()

add_subdirectory(src)
add_subdirectory(third_party)

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
