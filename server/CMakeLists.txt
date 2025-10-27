cmake_minimum_required(VERSION 3.16)
project(VideoTimelineServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(SOURCES
    server.cpp
)

# Create executable
add_executable(server ${SOURCES})

# Link Qt libraries
target_link_libraries(server
    Qt6::Core
    Qt6::Network
)

# Enable MOC
set_target_properties(server PROPERTIES
    AUTOMOC ON
)

# Installation
install(TARGETS server DESTINATION bin)

