include(FetchContent)

# Import JSON Library
FetchContent_Declare(json
  GIT_REPOSITORY https://github.com/ArthurSonzogni/nlohmann_json_cmake_fetchcontent
  GIT_TAG v3.9.1)
FetchContent_MakeAvailable(json)

# Import argparse library
FetchContent_Declare(argparse
  GIT_REPOSITORY https://github.com/p-ranav/argparse
  GIT_TAG master)
FetchContent_MakeAvailable(argparse)

# Import simple-websocket-server library
include(${CMAKE_CURRENT_SOURCE_DIR}/simplewebsocketserver.cmake)

set(battlesnakedownloader_SRCS
    main.cpp
    options.cpp
)

add_executable(battlesnakedownloader
    ${battlesnakedownloader_SRCS}
)

target_include_directories(battlesnakedownloader PRIVATE
    ${uuid_v4_SOURCE_DIR}
)

target_link_libraries(battlesnakedownloader simple-websocket-server)
target_link_libraries(battlesnakedownloader nlohmann_json::nlohmann_json)
target_link_libraries(battlesnakedownloader argparse)
target_link_libraries(battlesnakedownloader libbattlesnakerules)
target_link_libraries(battlesnakedownloader libbattlesnakejson)
