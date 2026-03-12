/ 808 Lo Tom — 80Hz, 640ms
/ N=28224
N: 28224
T: !N
E: e(T*(0-6.9%N))
/ small pitch sweep 125->80Hz
F: 80+45*e(T*(0-80%N))
D: F*(6.28318%44100)
P: +\D
S: (s P)
Q: e(T*(0-200%N))
R: r T
C: 0.06 f R
W: w (E*S)+(Q*C*.1)
