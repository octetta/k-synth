CC = gcc
CFLAGS = -O3 -Wall
LDFLAGS = -lm

all: ksynth

main.o : bestline.o miniaudio.o

bestline.o : bestline.c
	$(CC) -c bestline.c -o bestline.o

miniaudio.o : miniaudio.c
	$(CC) -c miniaudio.c -o miniaudio.o

STATIC_OBJS = bestline.o miniaudio.o

DEPS = ksynth.h
OBJS = ksynth.o main.o

ksynth: $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(STATIC_OBJS) $(LDFLAGS)

%.o: %.c $(DEPS)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f ksynth *.o
