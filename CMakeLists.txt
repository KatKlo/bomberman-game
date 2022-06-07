cmake_minimum_required(VERSION 3.1)
project(robots)

set(CMAKE_C_STANDARD 20)
set(CXX_STANDARD 20)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wconversion -Werror -O2 -std=gnu++20")

find_package(Boost 1.74.0 COMPONENTS program_options system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

add_library(Structures structures.h structures.cpp)

add_library(Parameters parameters.h parameters.cpp)
target_link_libraries(Parameters ${Boost_LIBRARIES})

add_library(Buffer buffer.h buffer.cpp)
target_link_libraries(Buffer Structures ${Boost_LIBRARIES})

add_library(GameState game_state.h game_state.cpp)
target_link_libraries(GameState Structures)

add_library(Connections connections.h connections.cpp)
target_link_libraries(Connections Buffer GameState ${Boost_LIBRARIES})

add_executable(robots-client robots-client.cpp)
add_executable(robots-server robots-server.cpp)

target_link_libraries(robots-client Parameters Structures Buffer GameState Connections ${Boost_LIBRARIES} -lpthread)
target_link_libraries(robots-server Parameters Structures Buffer GameState Connections ${Boost_LIBRARIES} -lpthread)


