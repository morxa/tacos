add_library(mtl_ata_translation SHARED translator.cpp)
target_link_libraries(mtl_ata_translation PRIVATE ta mtl)
#target_link_libraries(libta PRIVATE range-v3::range-v3)
target_include_directories(mtl_ata_translation PUBLIC include)
