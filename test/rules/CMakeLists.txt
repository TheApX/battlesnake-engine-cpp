set(testbattlesnakerules_SRCS
    standard_ruleset_test.cpp
    solo_ruleset_test.cpp
    royale_ruleset_test.cpp
    constrictor_ruleset_test.cpp
    squad_ruleset_test.cpp
    data_types_test.cpp
)

add_executable(testbattlesnakerules ${testbattlesnakerules_SRCS})

target_link_libraries(testbattlesnakerules
    libbattlesnakerules
    gtest_main
    gmock_main
)

add_test(NAME testbattlesnakerules
         COMMAND testbattlesnakerules)
