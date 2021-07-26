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
