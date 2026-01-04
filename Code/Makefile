CC = g++
CFLAGS = -std=c++17 -Wall $(shell pkg-config --cflags jsoncpp)
LDFLAGS = $(shell pkg-config --libs jsoncpp) # List source files here

SRCS = main.cpp
OBJECTS = $(SRCS:.cpp=.o)
EXECUTABLE = main

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

.cpp.o:
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(EXECUTABLE) $(OBJECTS)
