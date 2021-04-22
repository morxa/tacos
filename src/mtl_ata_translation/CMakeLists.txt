find_package(fmt REQUIRED)

add_library(mtl_ata_translation SHARED translator.cpp)
target_link_libraries(mtl_ata_translation PUBLIC ta mtl fmt::fmt)
target_include_directories(mtl_ata_translation PUBLIC include)
