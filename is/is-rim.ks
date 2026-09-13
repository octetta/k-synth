/ InSoc Rimshot — E-MU SP-12 "Rimshot" preset
/ SP-12 rimshot: static Chebyshev filter, 12-bit
/ Short, precise, tonal
/ 80ms
N: 3528
T: !N
E: e(T*(0-6.9%N))

F: 1700*((p 2)%(p 0))
G: 2300*((p 2)%(p 0))
P: +\(N#F)
Q: +\(N#G)
O: s P
I: s Q

S: (O*.6+I*.4)

/ SP-12 static filter on rimshot channel
C: 0.4 f S

V: e(T*(0-250%N))
R: r T
A: V*R*.4

Z: (E*C*.7)+(A*.4)
W: w 4096 v Z
