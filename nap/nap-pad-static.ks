/ NAP static pad — noise floor with tonal undertow
/ like a detuned radio through a broken amp
N: 44100
T: !N
/ sub tone cluster
F: 80*(6.28318%44100)
G: 81.5*(6.28318%44100)
H: 78.8*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
R: +\(N#H)
S: (s(P)+s(Q)+s(R))*0.333
/ noise layer at low level, adds static
X: r T
L: 0.08 f X
/ blend: mostly tone, some noise
V: S*0.75+L*0.35
C: d(V*2.5)
E: e(T*(0-1%N))
W: w E*C
