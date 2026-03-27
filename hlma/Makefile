EXE = \
  rtex4 \
  # rtex3 \
  # rtex2 \
  # rtex1 \
  # rt \
  # 16 \
  #

all: $(EXE)

OBJ = \
  simple_audio.o
  
rtex4: rtex4.c $(OBJ)
	$(CC) $(CFLAGS) -o $@ $< $(OBJ) -lm

rtex1: rtex1.c
	$(CC) $(CFLAGS) -o $@ $^ $(OBJ) -lm

rtex2: rtex2.c
	$(CC) $(CFLAGS) -o $@ $^ $(OBJ) -lm

rtex3: rtex3.c
	$(CC) $(CFLAGS) -o $@ $^ $(OBJ) -lm

simple_audio.o : simple_audio.c simple_audio.h
	$(CC) $(CFLAGS) -c $^

rt: real_time_wavetable_example.c simple_audio.o
	$(CC) $(CFLAGS) -o $@ $^ simple_audio.o -lm

16: 16_sine_voices_example.c
	$(CC) $(CFLAGS) -o $@ $^ -lm
