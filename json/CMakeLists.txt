include(FetchContent)

# Import JSON Library
FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
  GIT_TAG v3.9.1)
FetchContent_MakeAvailable(json)


set(libbattlesnakejson_SRCS
    converter.cpp
)

add_library(libbattlesnakejson STATIC
    ${libbattlesnakejson_SRCS}
)

target_include_directories(libbattlesnakejson PUBLIC
    ${BATTLESNAKE_ROOT_DIR}/include
)

target_link_libraries(libbattlesnakejson PUBLIC nlohmann_json::nlohmann_json)
target_link_libraries(libbattlesnakejson PUBLIC libbattlesnakerules)
