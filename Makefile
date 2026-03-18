CC = zig cc
CFLAGS = -O3 -Wall
LDFLAGS = -lm

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
	$(CC) -o test_ksynth test_ksynth.c ksynth.c -lm && ./test_ksynth

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f ksynth *.o
