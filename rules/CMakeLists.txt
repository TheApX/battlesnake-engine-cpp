include(FetchContent)

FetchContent_Declare(
  trivial_loop_array
  GIT_REPOSITORY https://github.com/TheApX/trivial-loop-array.git
  GIT_TAG        origin/main
)
FetchContent_MakeAvailable(trivial_loop_array)


set(libbattlesnakerules_SRCS
    data_types.cpp
    standard_ruleset.cpp
    solo_ruleset.cpp
    royale_ruleset.cpp
    constrictor_ruleset.cpp
    squad_ruleset.cpp
    helpers.cpp
)

add_library(libbattlesnakerules STATIC
    ${libbattlesnakerules_SRCS}
)

target_include_directories(libbattlesnakerules PUBLIC ${BATTLESNAKE_ROOT_DIR}/include)

target_link_libraries(libbattlesnakerules LINK_PUBLIC trivial_loop_array)
