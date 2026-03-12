/ NAP scream lead — high, nasty, overtone-rich, like feedback
/ C5 = 523.2Hz, aggressive filter, hard drive
N: 22050
T: !N
F: 523.2*(6.28318%44100)
G: 527.0*(6.28318%44100)
H: 519.5*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
R: +\(N#H)
A: 1 0.5 0.333 0.25 0.2 0.167 0.143
S: P $ A
U: Q $ A
X: R $ A
V: (S+U+X)*0.333
/ filter sweep: open → nasty resonant zone
O: e(T*(0-10%N))
B: 0.5 f V
I: 0.25 f V
D: (O*B)+((1-O)*I)
/ heavy drive
C: d(D*5.0)
E: e(T*(0-3%N))
W: w E*C
