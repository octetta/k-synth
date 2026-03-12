/ NAP metal perc — clanging inharmonic hit, like striking a pipe
/ inharmonic ratios: 1.0, 1.57, 2.35, 3.83 (real metal)
N: 11025
T: !N
A: 320*(6.28318%44100)
B: 502*(6.28318%44100)
C: 752*(6.28318%44100)
D: 1226*(6.28318%44100)
P: +\(N#A)
Q: +\(N#B)
R: +\(N#C)
S: +\(N#D)
/ different decay rates per partial
E: e(T*(0-12%N))
F: e(T*(0-20%N))
G: e(T*(0-35%N))
H: e(T*(0-55%N))
M: E*s(P)+F*s(Q)*0.7+G*s(R)*0.5+H*s(S)*0.3
/ noise burst transient
V: e(T*(0-300%N))
X: r T
K: V*(X-(0.1 f X))
W: w d((M*0.8+K*0.3)*1.8)
