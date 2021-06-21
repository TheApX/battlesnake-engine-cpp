set(libbattlesnakerules_SRCS
    ruleset.cpp
    standard_ruleset.cpp
)

add_library(libbattlesnakerules STATIC
    ${libbattlesnakerules_SRCS}
)

target_include_directories(libbattlesnakerules PUBLIC
    ${BATTLESNAKE_ROOT_DIR}/include
)
