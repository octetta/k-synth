/ InSoc Big Crash — S-900 user sample quality
/ The Akai S-900 sampler ran at full 44.1kHz, 16-bit — much cleaner than SP-12
/ InSoc layered S-900 samples on top of SP-12 for big hits
/ This represents the S-900 cymbal: cleaner, less gritty, more accurate
/ No quantize to 4096 — the S-900 was 16-bit, much cleaner
/ 1200ms
N: 52920
T: !N
E: e(T*(0-6.9%N))

B: 2800*(6.28318%44100)
P: +\(N#(B*1.000))
Q: +\(N#(B*1.342))
R: +\(N#(B*1.200))
S: +\(N#(B*1.618))
U: +\(N#(B*1.478))
A: 1 0 0.5 0 0.25
J: P $ A
K: Q $ A
L: R $ A
M: S $ A
X: U $ A
Z: J+K+L+M+X

C: m T
G: e(T*(0-5%N))

/ S-900: no quantize, clean signal
W: w (E*Z*.8)+(G*C*.3)
