include(${CMAKE_SOURCE_DIR}/simplewebserver.cmake)

set(libbattlesnakeserver_SRCS
    server.cpp
)

add_library(libbattlesnakeserver STATIC
    ${libbattlesnakeserver_SRCS}
)

target_include_directories(libbattlesnakeserver PUBLIC
    ${BATTLESNAKE_ROOT_DIR}/include
)

target_link_libraries(libbattlesnakeserver PUBLIC simple-web-server)
target_link_libraries(libbattlesnakeserver PUBLIC nlohmann_json::nlohmann_json)
