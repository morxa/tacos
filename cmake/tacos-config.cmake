include(CMakeFindDependencyMacro)
find_dependency(NamedType)
find_dependency(tinyxml2)
find_dependency(fmt)
find_dependency(spdlog)
find_dependency(range-v3)
find_dependency(PkgConfig)
pkg_check_modules(gvc libgvc QUIET IMPORTED_TARGET)
include("${CMAKE_CURRENT_LIST_DIR}/tacos-targets.cmake")