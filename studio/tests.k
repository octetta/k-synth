/ Sine Wave test (440Hz)
s (2 * p * 440 / 44100) * ! 10
/ Clipping and Logic test
0.5 & 1 | ! 5
/ Reshape and Scan composition
\ 5 # 1 2
