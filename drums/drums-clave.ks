/ 808 Clave — 2500Hz dominant, 100ms
/ N=4410
N: 4410
T: !N
E: e(T*(0-6.9%N))
F: 2500*((p 2)%(p 0))
G: 2750*((p 2)%(p 0))
P: +\(N#F)
Q: +\(N#G)
S: (s P)+(s Q)*.5
/ tiny noise snap
V: e(T*(0-30%N))
R: r T
W: w (E*S)+(V*R*.15)
