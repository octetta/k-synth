/ 808 Mid Tom — 122Hz, 500ms
/ N=22050
N: 22050
T: !N
E: e(T*(0-6.9%N))
/ small pitch sweep 165->122Hz
F: 122+43*e(T*(0-80%N))
D: F*(6.28318%44100)
P: +\D
S: (s P)
Q: e(T*(0-200%N))
R: r T
C: 0.08 f R
W: w (E*S)+(Q*C*.08)
