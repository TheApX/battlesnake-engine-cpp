include(FetchContent)

# Import JSON Library
FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
  GIT_TAG v3.9.1)
FetchContent_MakeAvailable(json)

set(libbattlesnakegameplayer_SRCS
    game_player.cpp
)

add_library(libbattlesnakegameplayer STATIC
    ${libbattlesnakegameplayer_SRCS}
)

target_include_directories(libbattlesnakegameplayer PUBLIC ${BATTLESNAKE_ROOT_DIR}/include)

target_link_libraries(libbattlesnakegameplayer LINK_PUBLIC libbattlesnakerules)
target_link_libraries(libbattlesnakegameplayer LINK_PUBLIC libbattlesnakeinterface)
target_link_libraries(libbattlesnakegameplayer LINK_PUBLIC libbattlesnakejson)
target_link_libraries(libbattlesnakegameplayer LINK_PUBLIC nlohmann_json::nlohmann_json)
