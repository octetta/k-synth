/ NAP dread pad — slow, suffocating, detuned and distorted
/ like a choir drowning in tar
N: (p 0)
T: !N
/ three heavily detuned voices
F: 130.8*((p 2)%(p 0))
G: 132.5*((p 2)%(p 0))
H: 129.2*((p 2)%(p 0))
P: +\(N#F)
Q: +\(N#G)
R: +\(N#H)
A: 1 0.5 0.333 0.25 0.2
S: P $ A
U: Q $ A
X: R $ A
V: (S+U+X)*0.333
/ very tight LP — muffled, dark
C: 0.06 f V
B: 0.06 f C
/ slight drive for grit
D: d(B*2.0)
E: e(T*(0-2%N))
W: w E*D
