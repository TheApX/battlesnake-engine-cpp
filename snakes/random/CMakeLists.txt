set(battlesnake_random_SRCS
    snake_random.cpp
    main.cpp
)

add_executable(battlesnake_random ${battlesnake_random_SRCS})

target_link_libraries(battlesnake_random
    libbattlesnakeinterface
    libbattlesnakeserver
    libbattlesnakejson
    simple-web-server
)


set(battlesnake_random_test_SRCS
    snake_random.cpp
    snake_random_test.cpp
)

add_executable(battlesnake_random_test ${battlesnake_random_test_SRCS})

target_link_libraries(battlesnake_random_test
    libbattlesnakeinterface
    gtest_main
    gmock_main
)

add_test(NAME battlesnake_random_test
         COMMAND battlesnake_random_test)
