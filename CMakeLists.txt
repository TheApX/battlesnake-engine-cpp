cmake_minimum_required(VERSION 3.14)

project(battlesnake-engine-cpp)

set(CMAKE_CXX_STANDARD 14)

# Suppress long and annoying warning.
SET(GCC_NO_PSABI_WARNING "-Wno-psabi")
add_definitions(${GCC_NO_PSABI_WARNING})

enable_testing()
add_subdirectory(test)


set(libbattlesnakerules_SRCS
    src/ruleset.cpp
    src/standard_ruleset.cpp
)

add_library(libbattlesnakerules STATIC
    ${libbattlesnakerules_SRCS}
)

target_include_directories(libbattlesnakerules PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
