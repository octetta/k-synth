/ NIN Lo Tom — 70Hz, heavy, room-saturated
N: 26460
T: !N
E: e(T*(0-6.9%N))
F: 70+60*e(T*(0-80%N))
D: F*(6.28318%44100)
P: +\D
O: s P
S: d(O*2)
V: e(T*(0-100%N))
R: r T
C: 0.08 f R
W: w (E*S*.85)+(V*C*.2)
