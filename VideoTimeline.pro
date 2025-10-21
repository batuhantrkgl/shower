QT       += core gui widgets multimedia multimediawidgets network

TARGET = VideoTimeline
TEMPLATE = app

# The C++ standard to use.
CONFIG += c++11

# --- Build Output Directory ---
DESTDIR = out
OBJECTS_DIR = out/obj
MOC_DIR = out/moc
RCC_DIR = out/rcc
UI_DIR = out/ui

# --- Start of Versioning Edit ---

# Base version number
VERSION = 1.1.0

# --- FIX: Use space-free ISO 8601 format to avoid shell parsing errors ---
# This command is much more reliable across different shells.
DATE_OUTPUT = $$system(date -u --iso-8601=seconds, read)
BUILD_DATE = $$join(DATE_OUTPUT, " ")

# The build ID command is fine, but we'll use the same robust method for consistency.
ID_OUTPUT = $$system(date +%s, read)
BUILD_ID = $$join(ID_OUTPUT, " ")
# --- End of FIX ---

# Pass the version info to the C++ preprocessor as macros.
DEFINES += APP_VERSION=\\\"$$VERSION\\\"
DEFINES += APP_RELEASE_DATE=\\\"$$BUILD_DATE\\\"
DEFINES += APP_BUILD_ID=\\\"$$BUILD_ID\\\"

# --- End of Versioning Edit ---

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    videowidget.cpp \
    timelinewidget.cpp \
    networkclient.cpp \
    mediaplayer.cpp

HEADERS += \
    mainwindow.h \
    videowidget.h \
    timelinewidget.h \
    networkclient.h \
    md3colors.h \
    mediaplayer.h \
    qt6compat.h
