/ NAP gravel pad — mid-range scrape, dissonant chord
/ minor 2nd interval = maximum tension
N: 44100
T: !N
/ tritone + minor 2nd cluster = very tense
F: 220*(6.28318%44100)
G: 233*(6.28318%44100)
H: 311*(6.28318%44100)
I: 330*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
R: +\(N#H)
S: +\(N#I)
A: 1 0.5 0.333 0.25 0.2
U: P $ A
V: Q $ A
X: R $ A
Y: S $ A
M: (U+V+X+Y)*0.25
/ LP at low cutoff to keep it dark
C: 0.12 f M
B: d(C*2.0)
E: e(T*(0-2%N))
W: w E*B
