QT       += core gui widgets multimedia multimediawidgets network

TARGET = VideoTimeline
TEMPLATE = app

# The C++ standard to use.
CONFIG += c++11

# --- Build Output Directory ---
# Executable will be placed in the 'out' directory
DESTDIR = out

# Intermediate files will be placed in subdirectories of 'out'
OBJECTS_DIR = out/obj
MOC_DIR = out/moc
RCC_DIR = out/rcc
UI_DIR = out/ui

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    videowidget.cpp \
    timelinewidget.cpp \
    networkclient.cpp

HEADERS += \
    mainwindow.h \
    videowidget.h \
    timelinewidget.h \
    networkclient.h \
    md3colors.h
