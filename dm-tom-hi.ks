/ DM Hi Tom — higher metal resonance, ~180Hz
/ 400ms
N: 17640
T: !N
E: e(T*(0-6.9%N))

F: 175*(6.28318%44100)
P: +\(N#F)
O: (s P)

/ Inharmonic partial at ~390Hz (not 2x = 350, slightly sharp)
G: 390*(6.28318%44100)
Q: +\(N#G)
I: (s Q)*e(T*(0-12%N))

J: 175+55*e(T*(0-120%N))
D: J*(6.28318%44100)
R: +\D
B: s R

V: e(T*(0-300%N))
K: r T
C: 0.15 f K
A: V*C

W: w (E*B*.85)+(I*.3)+(A*.2)
