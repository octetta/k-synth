EXE = \
  rt \
  # 16 \
  #

all: $(EXE)

rt: real_time_wavetable_example.c
	$(CC) $(CFLAGS) -o $@ $^ -lm

16: 16_sine_voices_example.c
	$(CC) $(CFLAGS) -o $@ $^ -lm
