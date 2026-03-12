/ NAP acid bass — 303-style but nastier, overdriven resonant filter
/ C2, 200ms, filter sweeping hard
N: 8820
T: !N
F: 65.4*(6.28318%44100)
G: 65.9*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
/ sawtooth
A: 1 0.5 0.333 0.25 0.2 0.167 0.143
S: P $ A
U: Q $ A
V: (S+U)*0.5
/ filter sweep: tight to wide very fast
O: e(T*(0-15%N))
B: 0.04 f V
I: 0.5 f V
D: (O*B)+((1-O)*I)
/ overdrive the output
E: e(T*(0-8%N))
W: w d(E*D*4.0)
