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

# Import UUID library
FetchContent_Declare(stduuid
  GIT_REPOSITORY https://github.com/mariusbancila/stduuid
  GIT_TAG master)
FetchContent_MakeAvailable(stduuid)

set(battlesnakecli_SRCS
    main.cpp
    cli_options.cpp
)

add_executable(battlesnakecli
    ${battlesnakecli_SRCS}
)

target_include_directories(battlesnakecli PRIVATE
    ${uuid_v4_SOURCE_DIR}
)

target_link_libraries(battlesnakecli PUBLIC argparse)
target_link_libraries(battlesnakecli PUBLIC stduuid)
target_link_libraries(battlesnakecli PUBLIC libbattlesnakerules)
