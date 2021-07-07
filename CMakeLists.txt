cmake_minimum_required(VERSION 3.14)

project(battlesnake-engine-cpp)

set(CMAKE_CXX_STANDARD 14)

# Suppress long and annoying warning.
set(GCC_NO_PSABI_WARNING "-Wno-psabi")
add_definitions(${GCC_NO_PSABI_WARNING})

set(BATTLESNAKE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

enable_testing()
add_subdirectory(test)

add_subdirectory(rules)
add_subdirectory(json)
add_subdirectory(interface)
add_subdirectory(server)
add_subdirectory(cli)
add_subdirectory(gamedownloader)

add_subdirectory(snakes)
