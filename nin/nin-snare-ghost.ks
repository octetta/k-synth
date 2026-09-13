/ NIN Ghost Snare — a quieter, more distant version
/ Used for the ghost hits in complex patterns
/ Same character as main snare but shorter decay, less noise floor
N: 8820
T: !N
B: e(T*(0-50%N))
F: 185*((p 2)%(p 0))
G: 230*((p 2)%(p 0))
P: +\(N#F)
Q: +\(N#G)
O: s P
I: s Q
S: B*((O+I)*.5)
K: 320*((p 2)%(p 0))
J: +\(N#K)
M: s J
Y: S*M
D: d(Y*4)
E: e(T*(0-6.9%N))
R: r T
U: E*R
W: w (D*.6)+(U*.3)
