/ Kraftwerk Mid Tom — Simmons SDS-V
/ 250Hz -> 120Hz — the "beeewww" mid sweep
/ 550ms
N: 24255
T: !N
E: e(T*(0-6.9%N))

F: 120+130*e(T*(0-40%N))
D: F*(6.28318%44100)
P: +\D
A: 1 0 -.111 0 .04 0 -.0204
S: P $ A

R: r T
L: 0.1 f R
U: e(T*(0-20%N))*L

V: e(T*(0-600%N))
K: r T
C: V*K

O: d(S*1.2)
W: w (E*O*.9)+(U*.12)+(C*.2)
