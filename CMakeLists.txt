cmake_minimum_required(VERSION 3.1)
project(robots)

set(CMAKE_C_STANDARD 20)
set(CXX_STANDARD 20)
set(CMAKE_C_FLAGS "-Wall -Wextra -Wconversion -Werror -O2 -g -std=gnu++20")
# -D CMAKE_CXX_COMPILER=/opt/gcc-11.2/bin/g++-11.2

find_package(Boost 1.74.0 COMPONENTS program_options system REQUIRED)
include_directories(${Boost_INCLUDE_DIR})

set(SOURCE_FILES
        robots-client.cpp
        parameters.h
        parameters.cpp
        structures.h
        structures.cpp
        utils.h
        utils.cpp
        buffer.h
        buffer.cpp
        game_state.cpp
        game_state.h
        connections.cpp
        connections.h
        )


add_executable(robots-client ${SOURCE_FILES})

#target_link_libraries(robots-client PUBLIC -lboost_system -lboost_program_options -lpthread -lstdc++)
target_link_libraries(robots-client ${Boost_LIBRARIES} -lpthread)


