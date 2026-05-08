CC := zig cc
TEST_CC ?= gcc
CFLAGS = -O3 -Wall
LDFLAGS = -lm

.PHONY: all test wasm clean

all: ksynth

main.o : bestline.o miniaudio.o kgnuplot.o

bestline.o : bestline.c
	$(CC) -c bestline.c -o bestline.o

miniaudio.o : miniaudio.c
	$(CC) -c miniaudio.c -o miniaudio.o

kgnuplot.o : kgnuplot.c
	$(CC) -c kgnuplot.c -o kgnuplot.o

STATIC_OBJS = bestline.o miniaudio.o kgnuplot.o

DEPS = ksynth.h
OBJS = ksynth.o main.o

ksynth: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(STATIC_OBJS) $(LDFLAGS)

test: test_ksynth.c ksynth.c ksynth.h
	$(TEST_CC) -O3 -Wall -o test_ksynth test_ksynth.c ksynth.c -lm && ./test_ksynth

wasm: build.sh ksynth.c ks_api.c ksynth.h docs-build.py guide.md readme.md reference.md api.md
	./build.sh

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f ksynth *.o
