/ Kraftwerk Lo Tom — Simmons SDS-V
/ 140Hz -> 55Hz — the deep sweep, almost a bass note
/ 650ms
N: 28665
T: !N
E: e(T*(0-6.9%N))

F: 55+85*e(T*(0-35%N))
D: F*(6.28318%44100)
P: +\D
A: 1 0 -.111 0 0.04 0 -.0204
S: P $ A

R: r T
L: 0.06 f R
U: e(T*(0-20%N))*L

V: e(T*(0-600%N))
K: r T
C: V*K

O: d(S*1.2)
W: w (E*O*.9)+(U*.1)+(C*.2)
