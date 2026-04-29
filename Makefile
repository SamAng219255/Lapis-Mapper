CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude -Ilib
CFLAGS += $(shell pkg-config --cflags libpng)
LDLIBS = $(shell pkg-config --libs libpng zlib)

SRC = $(wildcard src/*.c) $(wildcard lib/*.c)
OBJ = $(patsubst %.c,build/%.o,$(SRC))

TARGET = bin/lapis

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p bin
	$(CC) $(OBJ) -o $@ $(LDLIBS)

build/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build bin

.PHONY: all clean