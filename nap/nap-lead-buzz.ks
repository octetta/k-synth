/ NAP buzz lead — raw sawtooth with no filter mercy
/ C4, completely unfiltered, just harmonics and grime
N: 22050
T: !N
F: 261.6*(6.28318%44100)
G: 263.5*(6.28318%44100)
H: 259.8*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
R: +\(N#H)
/ 10 harmonics — full raw sawtooth
A: 1 0.5 0.333 0.25 0.2 0.167 0.143 0.125 0.111 0.1
S: P $ A
U: Q $ A
X: R $ A
V: (S+U+X)*0.333
/ light tanh just for protection, not for warmth
B: d(V*1.5)
E: e(T*(0-4%N))
W: w E*B
