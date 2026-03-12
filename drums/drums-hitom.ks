/ 808 Hi Tom — 170Hz, 500ms
/ N=22050
N: 22050
T: !N
E: e(T*(0-6.9%N))
/ small pitch sweep 212->170Hz
F: 170+42*e(T*(0-80%N))
D: F*(6.28318%44100)
P: +\D
S: (s P)
/ short thump
Q: e(T*(0-200%N))
R: r T
C: 0.1 f R
W: w (E*S)+(Q*C*.08)
