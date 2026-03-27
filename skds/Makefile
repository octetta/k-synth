CC = gcc
#CFLAGS = -Wall -Wextra -O3 -std=c99 -Wno-stringop-overflow
CFLAGS = -Wall -Wextra -Ofast -march=native -ffast-math -Wno-stringop-overflow -std=c99
LDFLAGS = -lm -lpthread -ldl

TARGET = skred_demo
OBJS = main.o skred_ds.o miniaudio.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(OBJS) $(LDFLAGS)

main.o: main.c skred_ds.h miniaudio.h
	$(CC) $(CFLAGS) -c main.c

skred_ds.o: skred_ds.c skred_ds.h miniaudio.h
	$(CC) $(CFLAGS) -c skred_ds.c

miniaudio.o: miniaudio.c miniaudio.h
	$(CC) $(CFLAGS) -c miniaudio.c

clean:
	rm -f $(OBJS) $(TARGET)
