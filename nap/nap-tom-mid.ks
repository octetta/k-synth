/ NAP mid tom — cracked, metallic thwack
N: 13230
T: !N
E: e(T*(0-8%N))
F: 120+80*e(T*(0-50%N))
D: F*(6.28318%44100)
P: +\D
S: d(s(P)*4.5)
/ metallic ring layer
A: 340*(6.28318%44100)
Q: +\(N#A)
M: e(T*(0-20%N))*s(Q)
R: r T
V: e(T*(0-300%N))
K: V*(R-(0.06 f R))
W: w (E*S*0.6+M*0.3+K*0.3)
