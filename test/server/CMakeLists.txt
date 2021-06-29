include(${CMAKE_SOURCE_DIR}/simplewebserver.cmake)

file(GLOB SRCS *.cpp)

set(testbattlesnakeserver_SRCS
    server_test.cpp
)

add_executable(testbattlesnakeserver ${testbattlesnakeserver_SRCS})

target_link_libraries(testbattlesnakeserver
    libbattlesnakeserver
    libbattlesnakejson
    gtest_main
    gmock_main
    simple-web-server
)

add_test(NAME testbattlesnakeserver
         COMMAND testbattlesnakeserver)
