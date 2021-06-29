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
