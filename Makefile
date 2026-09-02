CC := gcc
CFLAGS := -Wall -Wextra -Werror -std=c11 -O2 -fopenmp -pthread
LDLIBS := -lm
TARGET := mandelbrot
SOURCES := src/main.c src/mandelbrot.c
OBJECTS := $(SOURCES:.c=.o)

.PHONY: all clean test

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(OBJECTS) -o $@ $(LDLIBS)

src/%.o: src/%.c src/mandelbrot.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJECTS) mandelbrot_bgs_*.pgm times.txt

test: $(TARGET)
	sh tests/test.sh
