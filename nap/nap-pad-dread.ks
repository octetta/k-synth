/ NAP dread pad — slow, suffocating, detuned and distorted
/ like a choir drowning in tar
N: 44100
T: !N
/ three heavily detuned voices
F: 130.8*(6.28318%44100)
G: 132.5*(6.28318%44100)
H: 129.2*(6.28318%44100)
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
