cmake_minimum_required(VERSION 2.8)

project(battlesnake-engine-cpp)

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

target_include_directories(libbattlesnakerules INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
