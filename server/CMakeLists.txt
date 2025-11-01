cmake_minimum_required(VERSION 3.16)

project(server)

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Network)

set(SOURCES
    server.cpp
)

set(HEADERS
    server.h
)

# Create executable
add_executable(server ${SOURCES} ${HEADERS})

# Link Qt libraries
target_link_libraries(server
    Qt6::Core
    Qt6::Network
)

# Enable MOC
set_target_properties(server PROPERTIES
    AUTOMOC ON
)

# Set data directory path
target_compile_definitions(server PRIVATE
    DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../data"
    MEDIA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/../data/media"
)