file(GLOB SRCS *.cpp)

add_executable(testbattlesnakerules ${SRCS})

target_link_libraries(testbattlesnakerules
    libbattlesnakerules
    gtest_main
    gmock_main
)

add_test(NAME testbattlesnakerules
         COMMAND testbattlesnakerules)
