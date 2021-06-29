include(FetchContent)

# Import asio Library
set(ASIO_CMAKE_ASIO_SOURCE_DIR ${CMAKE_BINARY_DIR}/_deps)
FetchContent_Declare(asio-cmake
  GIT_REPOSITORY https://github.com/kingsamchen/asio-cmake.git
  GIT_TAG        origin/master
)
set(ASIO_CMAKE_ASIO_TAG asio-1-18-2)
FetchContent_MakeAvailable(asio-cmake)

set(ASIO_PATH ${ASIO_CMAKE_ASIO_SOURCE_DIR}/asio-${ASIO_CMAKE_ASIO_TAG}-src/asio/include)

# Import simple-web-server Library
option(USE_STANDALONE_ASIO "-" ON)
FetchContent_Declare(simple-web-server
  GIT_REPOSITORY https://gitlab.com/eidheim/Simple-Web-Server.git
  GIT_TAG v3.1.1)
FetchContent_MakeAvailable(simple-web-server)

target_include_directories(simple-web-server INTERFACE
    ${ASIO_PATH}
)
