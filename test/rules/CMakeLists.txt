file(GLOB SRCS *.cpp)

set(testbattlesnakerules_SRCS
    standard_ruleset_test.cpp
    solo_ruleset_test.cpp
    royale_ruleset_test.cpp
    constrictor_ruleset_test.cpp
    squad_ruleset_test.cpp
)

add_executable(testbattlesnakerules ${testbattlesnakerules_SRCS})

target_link_libraries(testbattlesnakerules
    libbattlesnakerules
    gtest_main
    gmock_main
)

add_test(NAME testbattlesnakerules
         COMMAND testbattlesnakerules)


SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg")
SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")


set(testbattlesnakerulesperf_SRCS
    royale_ruleset_perftest.cpp
)

add_executable(testbattlesnakerulesperf ${testbattlesnakerulesperf_SRCS})

target_link_libraries(testbattlesnakerulesperf
    libbattlesnakerules
)
