/ NIN Hi Tom — 190Hz, punchy, distorted
N: 17640
T: !N
E: e(T*(0-6.9%N))
F: 190+90*e(T*(0-100%N))
D: F*(6.28318%44100)
P: +\D
O: s P
S: d(O*2)
V: e(T*(0-150%N))
R: r T
C: 0.15 f R
W: w (E*S*.8)+(V*C*.2)
