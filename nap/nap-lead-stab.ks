/ NAP synth stab — vicious short stab, think NIN Closer
/ short gate, overdriven, filter slammed open then shut
N: 5512
T: !N
F: 196*(6.28318%44100)
G: 197.5*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
A: 1 0.5 0.333 0.25 0.2 0.167
S: P $ A
U: Q $ A
V: (S+U)*0.5
/ filter snaps open fast then gone
O: e(T*(0-50%N))
B: 0.08 f V
I: 0.55 f V
D: (O*I)+((1-O)*B)
C: d(D*4.0)
E: e(T*(0-20%N))
W: w E*C
