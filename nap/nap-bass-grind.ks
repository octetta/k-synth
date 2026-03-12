/ NAP grind bass — square wave grind, brutal low-mid
/ heavily clipped, almost fuzz bass
N: 11025
T: !N
F: 55*(6.28318%44100)
G: 55.4*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
/ square wave approximation: odd harmonics
A: 1 0 0.333 0 0.2 0 0.143 0 0.111
S: P $ A
U: Q $ A
V: (S+U)*0.5
/ fuzz: clip very hard
B: d(V*8.0)
/ LP to tame the harshest stuff
C: 0.2 f B
E: e(T*(0-5%N))
W: w E*C
