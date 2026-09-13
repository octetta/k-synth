/ Mid Tom
/ 600ms at (p 0) = 26460 samples

N: 26460
T: !N
E: e(T*(0-6.9%N))
F: 110+90*e(T*(0-120%N))
D: F*((p 2)%(p 0))
P: +\D
S: (s P)
Q: e(T*(0-220%N))
R: r T
C: 0.1 f R
W: w (E*S)+(Q*C*.08)
