/ NAP low tom — brutal, overdriven, loose
N: 22050
T: !N
E: e(T*(0-5%N))
F: 50+50*e(T*(0-30%N))
D: F*(6.28318%44100)
P: +\D
S: s P
B: d(S*5.0)
R: r T
L: 0.12 f R
G: e(T*(0-8%N))
W: w (E*B*0.8+G*L*0.25)
