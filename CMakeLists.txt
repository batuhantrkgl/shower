cmake_minimum_required(VERSION 3.16)
project(VideoTimeline)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Optimize compilation flags for low-RAM systems and avoid GCC ICE crashes
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG" CACHE STRING "Flags used by the CXX compiler during Release builds" FORCE)
endif()

# Find Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Multimedia MultimediaWidgets)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Include directories
include_directories(src)
include_directories(data)

# Set RPATH for relocatable installs
set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
set(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE)

# Build main application
add_subdirectory(src)

# Build server
add_subdirectory(server)