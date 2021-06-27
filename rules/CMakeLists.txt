set(libbattlesnakerules_SRCS
    data_types.cpp
    standard_ruleset.cpp
    solo_ruleset.cpp
    royale_ruleset.cpp
    constrictor_ruleset.cpp
    helpers.cpp
)

add_library(libbattlesnakerules STATIC
    ${libbattlesnakerules_SRCS}
)

target_include_directories(libbattlesnakerules PUBLIC
    ${BATTLESNAKE_ROOT_DIR}/include
)
