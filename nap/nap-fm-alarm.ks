/ NAP FM alarm — two-op FM with high index, harsh and strident
/ carrier 880Hz, modulator 660Hz, ratio 3:2 = inharmonic spread
N: 22050
T: !N
C: 880*(6.28318%44100)
M: 660*(6.28318%44100)
P: +\(N#C)
Q: +\(N#M)
/ high modulation index decaying fast = bright attack, harsh sustain
I: 8.0*e(T*(0-30%N))
/ FM: carrier phase modulated by index*mod
/ use P+(I*s(Q)) as phase input to sin
/ can't do sin(P+I*sin(Q)) directly so approximate:
/ S = sin(P)*cos(I*sin(Q)) + cos(P)*sin(I*sin(Q))
/ simplified: since I is large, just slam sin(Q) into filter as modulator
S: s Q
B: 0.5+0.4*S
/ sawtooth at carrier, filter modulated by mod signal
A: 1 0.5 0.333 0.25 0.2 0.167
U: P $ A
/ FM-style timbral modulation: use mod signal to open/close filter
D: (B*0.7) f U
E: e(T*(0-5%N))
W: w d(E*D*2.5)
