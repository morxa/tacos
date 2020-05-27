cmake_minimum_required (VERSION 3.8)

# projectname is the same as the main-executable
project(mtlsyn)

include(GNUInstallDirs)
include(CTest)

add_compile_options(-W -Wall -Werror)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(src)

if (BUILD_TESTING)
  add_subdirectory(test)
endif()
