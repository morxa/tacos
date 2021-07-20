add_library(xmlWriter XmlWriter.cpp)
target_include_directories(xmlWriter INTERFACE include)


add_dependencies(xmlWriter tinyxml2)
target_link_libraries(xmlWriter PUBLIC tinyxml2)