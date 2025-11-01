set(SOURCES
    main.cpp
    mainwindow.cpp
    videowidget.cpp
    timelinewidget.cpp
    activityoverlay.cpp
    networkclient.cpp
    mediaplayer.cpp
    statusbar.cpp
)

set(HEADERS
    mainwindow.h
    videowidget.h
    timelinewidget.h
    activityoverlay.h
    networkclient.h
    md3colors.h
    mediaplayer.h
    qt6compat.h
    statusbar.h
)

set(RESOURCES
    icons.qrc
)

# Create executable
add_executable(VideoTimeline ${SOURCES} ${HEADERS} ${RESOURCES})

# Link Qt libraries
target_link_libraries(VideoTimeline
    Qt6::Core
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::MultimediaWidgets
    Qt6::Network
)

# Enable MOC, RCC
set_target_properties(VideoTimeline PROPERTIES
    AUTOMOC ON
    AUTORCC ON
)

# Version information
set(VERSION "1.1.0")
string(TIMESTAMP BUILD_DATE "%Y-%m-%dT%H:%M:%SZ" UTC)
string(TIMESTAMP BUILD_ID "%s" UTC)

target_compile_definitions(VideoTimeline PRIVATE
    APP_VERSION="${VERSION}"
    APP_RELEASE_DATE="${BUILD_DATE}"
    APP_BUILD_ID="${BUILD_ID}"
)