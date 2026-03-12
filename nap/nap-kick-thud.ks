/ NAP kick — heavy thud with sub and grit, distorted body
/ deep 808-style but nastier — saturated sub, noise transient, pitch snap
N: 17640
T: !N
E: e(T*(0-5%N))

/ pitch envelope: 200Hz snap down to 40Hz
F: 40+160*e(T*(0-80%N))
D: F*(6.28318%44100)
P: +\D
S: s P

/ drive the sine hard through tanh for grit
B: d(S*3.0)

/ sub layer: pure sine at 40Hz
G: 40*(6.28318%44100)
M: +\(N#G)
O: s M
U: d(O*1.5)

/ noise click transient
V: e(T*(0-400%N))
R: r T
C: R-(0.05 f R)
K: V*C

W: w (E*B*0.7)+(E*U*0.4)+(K*0.3)
