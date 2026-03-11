/ Crash Cymbal
/ 1200ms at 44100 = 52920 samples
/ Six inharmonic oscillators — no lowpass, keep metallic highs

N: 52920
T: !N
E: e(T*(0-6.9%N))
B: 450*(6.28318%44100)
P: +\(N#(B*1.000))
Q: +\(N#(B*1.342))
R: +\(N#(B*1.200))
S: +\(N#(B*1.618))
U: +\(N#(B*1.478))
V: +\(N#(B*1.784))
A: 1 0 0.33 0 0.2 0 0.14
J: P $ A
K: Q $ A
L: R $ A
M: S $ A
X: U $ A
Y: V $ A
Z: J+K+L+M+X+Y
/ noise shimmer — r T for correct length
C: r T
G: e(T*(0-6.9%N))
W: w E*(Z+G*C*.3)
