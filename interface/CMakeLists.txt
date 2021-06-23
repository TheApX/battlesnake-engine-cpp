include(FetchContent)

# Import JSON Library
FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
  GIT_TAG v3.9.1)
FetchContent_MakeAvailable(json)


set(libbattlesnakeinterface_SRCS
    interface.cpp
)

add_library(libbattlesnakeinterface STATIC
    ${libbattlesnakeinterface_SRCS}
)

target_include_directories(libbattlesnakeinterface PUBLIC
    ${BATTLESNAKE_ROOT_DIR}/include
)

target_link_libraries(libbattlesnakeinterface PUBLIC libbattlesnakerules)
