# Required by GTest&GMock
find_package(Threads REQUIRED)

include(FetchContent)

FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/master.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

file(GLOB SRCS *.cpp)

add_executable(testbattlesnakerules ${SRCS})

target_link_libraries(testbattlesnakerules
    libbattlesnakerules
    gtest_main
    gmock_main
)

add_test(NAME testbattlesnakerules
         COMMAND testbattlesnakerules)

