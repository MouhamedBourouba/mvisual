CC = clang
OUTPUT = mvisual

SOURCES = $(wildcard src/*.c)
LIBS =  $(shell pkg-config --libs raylib) -lm
CFLAGS = -ggdb -Wall -Wextra -I include/ $(shell pkg-config --cflags raylib)

all: $(OUTPUT)

$(OUTPUT): $(SOURCES)
	 $(CC) -o $@ $(CFLAGS) $^ $(LIBS)

clean: rm $(OUTPUT)
