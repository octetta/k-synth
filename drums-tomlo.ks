/ Low Tom
/ 700ms at 44100 = 30870 samples

N: 30870
T: !N
E: e(T*(0-6.9%N))
F: 55+75*e(T*(0-140%N))
D: F*(6.28318%44100)
P: +\D
S: (s P)
Q: e(T*(0-220%N))
R: r T
C: 0.08 f R
W: w E*S+Q*C*.1
