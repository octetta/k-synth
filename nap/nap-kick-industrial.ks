/ NAP kick — industrial stomp, metallic body with sub
N: 19845
T: !N
E: e(T*(0-4%N))

/ low thud with fast pitch sweep
F: 55+220*e(T*(0-60%N))
D: F*(6.28318%44100)
P: +\D
S: d(s(P)*4.0)

/ metallic ring layer
A: 180*(6.28318%44100)
B: 271*(6.28318%44100)
Q: +\(N#A)
R: +\(N#B)
M: (s(Q)+s(R))*0.5
MC: e(T*(0-25%N))

/ noise slam
X: r T
NC: e(T*(0-200%N))
NF: 0.3 f X

W: w (E*S*0.6)+(MC*d(M*2.5)*0.35)+(NC*NF*0.3)
