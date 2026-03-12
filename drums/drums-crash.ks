/ 808 Crash — 6kHz dominant, 1500ms, inharmonic
/ N=66150
N: 66150
T: !N
E: e(T*(0-6.9%N))
/ inharmonic oscillators in high register
/ 808 crash ratios scaled to ~3kHz base
B: 3000*(6.28318%44100)
P: +\(N#(B*1.000))
Q: +\(N#(B*1.342))
R: +\(N#(B*1.200))
S: +\(N#(B*1.618))
U: +\(N#(B*1.478))
/ harmonic content: odd partials for metallic character
A: 1 0 0.5 0 0.25
J: P $ A
K: Q $ A
L: R $ A
M: S $ A
X: U $ A
Z: J+K+L+M+X
/ 1-bit noise shimmer
C: m T
G: e(T*(0-6.9%N))
W: w (E*Z*.7)+(G*C*.4)
