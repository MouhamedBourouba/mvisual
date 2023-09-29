CC = clang++
OUTPUT = mvisual

SOURCES = $(wildcard src/*.cpp)
LIBS =  $(shell pkg-config --libs raylib)
CFLAGS = -ggdb -Wall -Wextra -I include/ $(shell pkg-config --cflags raylib)

all: $(OUTPUT)

$(OUTPUT): $(SOURCES)
	 $(CC) -o $@ $(CFLAGS) $^ $(LIBS)

clean: rm $(OUTPUT)
