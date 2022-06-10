cmake_minimum_required(VERSION 3.1)
project(robots)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wconversion -Werror -O2 -std=gnu++20")

find_package(Boost 1.74.0 COMPONENTS program_options system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

set(COMMON
        parameters.h
        parameters.cpp
        structures.h
        structures.cpp
        connections/connections.h
        connections/connections.cpp
        game_managers/game_info.h
        game_managers/game_info.cpp
        )

set(BUFFERS
        buffers/buffer.h
        buffers/incoming_buffer.cpp
        buffers/incoming_buffer.h
        buffers/outgoing_buffer.cpp
        buffers/outgoing_buffer.h
        buffers/udp_incoming_buffer.cpp
        buffers/udp_incoming_buffer.h
        buffers/tcp_incoming_buffer.cpp
        buffers/tcp_incoming_buffer.h
        )

set(CLIENT_CONNECTIONS
        connections/client_connections.h
        connections/client_connections.cpp
        )

set(SERVER_CONNECTIONS
        connections/server_connections.h
        connections/server_connections.cpp
        )

set(CLIENT_GAME_INFO
        game_managers/client_game_info.h
        game_managers/client_game_info.cpp
        )

set(SERVER_GAME_INFO
        game_managers/server_game_info.h
        game_managers/server_game_info.cpp
        )

set(CLIENT
        robots-client.cpp
        ${COMMON}
        ${BUFFERS}
        ${CLIENT_GAME_INFO}
        ${CLIENT_CONNECTIONS}
        )

set(SERVER
        robots-server.cpp
        ${COMMON}
        ${BUFFERS}
        ${SERVER_GAME_INFO}
        ${SERVER_CONNECTIONS}
        )

add_executable(robots-client ${CLIENT})
add_executable(robots-server ${SERVER})

target_link_libraries(robots-client ${Boost_LIBRARIES} -lpthread)
target_link_libraries(robots-server ${Boost_LIBRARIES} -lpthread)
