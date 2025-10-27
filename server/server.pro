QT = core network

TARGET = server
TEMPLATE = app

CONFIG += moc

SOURCES += server.cpp
HEADERS += server.h

# Output directory
DESTDIR = .
OBJECTS_DIR = obj
MOC_DIR = moc
RCC_DIR = rcc

# C++17
CONFIG += c++17

# Debug/Release
CONFIG(release, debug|release) {
    DEFINES += NDEBUG
    QMAKE_CXXFLAGS += -O2
}

CONFIG(debug, debug|release) {
    QMAKE_CXXFLAGS += -g
    DEFINES += DEBUG
}

# Installation
target.path = /usr/local/bin
INSTALLS += target

