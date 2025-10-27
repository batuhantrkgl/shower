cmake_minimum_required(VERSION 3.16)

project(VideoTimeline VERSION 1.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 components
find_package(Qt6 REQUIRED COMPONENTS
    Core
    Widgets
    Multimedia
    MultimediaWidgets
    Network
)

# Enable automatic MOC, UIC, and RCC processing
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Set build output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/out)

# Version and build information
string(TIMESTAMP BUILD_DATE "%Y-%m-%dT%H:%M:%S" UTC)
string(TIMESTAMP BUILD_ID "%s")

# Define preprocessor macros for version info
add_compile_definitions(
    APP_VERSION="${PROJECT_VERSION}"
    APP_RELEASE_DATE="${BUILD_DATE}"
    APP_BUILD_ID="${BUILD_ID}"
)

# Source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    videowidget.cpp
    timelinewidget.cpp
    networkclient.cpp
    mediaplayer.cpp
)

# Header files
set(HEADERS
    mainwindow.h
    videowidget.h
    timelinewidget.h
    networkclient.h
    md3colors.h
    mediaplayer.h
    qt6compat.h
)

# Resource files
set(RESOURCES
    icons.qrc
)

# Create executable
add_executable(VideoTimeline
    ${SOURCES}
    ${HEADERS}
    ${RESOURCES}
)

# Link Qt6 libraries
target_link_libraries(VideoTimeline
    Qt6::Core
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::Network
)

# Compiler-specific options
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    target_compile_options(VideoTimeline PRIVATE -Wall -Wextra)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(VideoTimeline PRIVATE /W4)
endif()

# Platform-specific settings
if(WIN32)
    set_target_properties(VideoTimeline PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
elseif(APPLE)
    set_target_properties(VideoTimeline PROPERTIES
        MACOSX_BUNDLE TRUE
    )
endif()

# Install rules
install(TARGETS VideoTimeline
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Create build info
message(STATUS "VideoTimeline ${PROJECT_VERSION}")
message(STATUS "Build date: ${BUILD_DATE}")
message(STATUS "Build ID: ${BUILD_ID}")
message(STATUS "Qt6 version: ${Qt6_VERSION}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

# Qt6 compatibility checks
if(Qt6_VERSION VERSION_LESS "6.2.0")
    message(WARNING "Qt6 version ${Qt6_VERSION} may have compatibility issues. Recommended: 6.2.0 or later")
endif()

# Optional: Create a custom target for running the application
add_custom_target(run
    COMMAND VideoTimeline
    DEPENDS VideoTimeline
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Running VideoTimeline application"
)

# Optional: Create a custom target for cleaning output directory
add_custom_target(clean-output
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
    COMMENT "Cleaning output directory"
)
