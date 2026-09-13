/ NAP static pad — noise floor with tonal undertow
/ like a detuned radio through a broken amp
N: (p 0)
T: !N
/ sub tone cluster
F: 80*((p 2)%(p 0))
G: 81.5*((p 2)%(p 0))
H: 78.8*((p 2)%(p 0))
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
