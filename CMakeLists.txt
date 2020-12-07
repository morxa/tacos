cmake_minimum_required (VERSION 3.8)

# projectname is the same as the main-executable
project(mtlsyn)

include(GNUInstallDirs)
include(CTest)
option(MTLSYN_BUILD_DOC "Allow building documentation, requires doxygen." ON)

add_compile_options(-W -Wall -Werror)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)

find_package(Boost REQUIRED)

if (BUILD_TESTING)
  add_subdirectory(test)
endif()

if(${MTLSYN_BUILD_DOC})
	find_package(Doxygen REQUIRED dot)
	if (DOXYGEN_FOUND)
  		set(DOXYGEN_OUTPUT_DIRECTORY api)
  		set(DOXYGEN_WARN_AS_ERROR 1)
  		doxygen_add_docs(doc src)
	endif()
endif()
