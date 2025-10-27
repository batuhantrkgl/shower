# Makefile for VideoTimeline Server

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
QT_MODULES = Core Network
QT_CFLAGS = $(shell pkg-config --cflags Qt6$(shell echo $(QT_MODULES) | tr ' ' ','))
QT_LIBS = $(shell pkg-config --libs Qt6$(shell echo $(QT_MODULES) | tr ' ' ','))

TARGET = server
SOURCES = server.cpp
OBJ_DIR = obj
OBJS = $(OBJ_DIR)/server.o $(OBJ_DIR)/server.moc.o

# Create obj directory if it doesn't exist
$(shell mkdir -p $(OBJ_DIR))

.PHONY: all clean run

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(OBJS) -o $(TARGET) $(QT_LIBS)

$(OBJ_DIR)/server.o: server.cpp
	$(CXX) $(CXXFLAGS) $(QT_CFLAGS) -c $< -o $@

$(OBJ_DIR)/server.moc.o: $(OBJ_DIR)/server.cpp
	$(CXX) $(CXXFLAGS) $(QT_CFLAGS) -c $< -o $@

$(OBJ_DIR)/server.cpp: server.cpp
	moc $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) $(OBJ_DIR)/server.cpp $(OBJ_DIR)/*.o

run: $(TARGET)
	./$(TARGET)

install: $(TARGET)
	@echo "Installing server..."
	cp $(TARGET) /usr/local/bin/
	@echo "Server installed to /usr/local/bin/"

