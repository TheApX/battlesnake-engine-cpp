include(${BATTLESNAKE_ROOT_DIR}/simplewebserver.cmake)

file(GLOB SRCS *.cpp)

set(testbattlesnakeserver_SRCS
    server_test.cpp
)

add_executable(testbattlesnakeserver ${testbattlesnakeserver_SRCS})

target_link_libraries(testbattlesnakeserver
    libbattlesnakeserver
    libbattlesnakejson
    simple-web-server
    gtest_main
    gmock_main
)

add_test(NAME testbattlesnakeserver
         COMMAND testbattlesnakeserver)
