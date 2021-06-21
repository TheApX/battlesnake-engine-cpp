file(GLOB SRCS *.cpp)

set(testbattlesnakejson_SRCS
    create_json_test.cpp
    parse_json_test.cpp
)

add_executable(testbattlesnakejson ${testbattlesnakejson_SRCS})

target_link_libraries(testbattlesnakejson
    libbattlesnakejson
    gtest_main
    gmock_main
)

add_test(NAME testbattlesnakejson
         COMMAND testbattlesnakejson)
