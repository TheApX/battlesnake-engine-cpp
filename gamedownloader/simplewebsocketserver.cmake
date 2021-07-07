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

# Import simple-websocket-server Library
option(USE_STANDALONE_ASIO "-" ON)
FetchContent_Declare(simple-websocket-server
  GIT_REPOSITORY https://gitlab.com/eidheim/Simple-WebSocket-Server.git
  GIT_TAG v2.0.2)
FetchContent_MakeAvailable(simple-websocket-server)

target_include_directories(simple-websocket-server INTERFACE
    ${ASIO_PATH}
)
