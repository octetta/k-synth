/ Kraftwerk Clave / Woodblock — Maestro Rhythm-King
/ "Autobahn", early Kraftwerk rhythm box sounds
/ Very short, pitched high, almost like a triangle wave ping
/ 70ms
N: 3087
T: !N
E: e(T*(0-6.9%N))

/ High triangle tone
F: 2200*(6.28318%44100)
G: 2900*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
A: 1 0 -.111 0 .04
O: P $ A
I: Q $ A
S: O+I*.4

/ Very fast noise click underneath
V: e(T*(0-400%N))
R: r T
K: V*R

W: w E*(S*.8+K*.3)
