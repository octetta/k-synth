/ NAP snare — violent crack, thin body, nasty noise burst
N: 8820
T: !N
A: 180*(6.28318%44100)
B: 330*(6.28318%44100)
P: +\(N#A)
Q: +\(N#B)
/ body envelope
C: e(T*(0-40%N))
S: C*(s(P)*0.6+s(Q)*0.4)
/ noise
R: r T
E: e(T*(0-9%N))
L: 0.6 f R
/ crack transient HP
X: r T
V: e(T*(0-500%N))
K: V*(X-(0.04 f X))
W: w d((S*0.4+E*L*0.7+K*0.5)*2.0)
