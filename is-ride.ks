/ InSoc Ride — E-MU SP-12 "Ride" preset
/ Same unfiltered channel as crash, shorter transient character
/ Used in patterns for rhythmic metal texture
/ 800ms
N: 35280
T: !N
E: e(T*(0-6.9%N))

B: 4000*(6.28318%44100)
P: +\(N#(B*1.000))
Q: +\(N#(B*1.342))
R: +\(N#(B*1.480))
S: +\(N#(B*1.618))
A: 1 0 0.4 0 0.2
J: P $ A
K: Q $ A
L: R $ A
M: S $ A
Z: J+K+L+M

C: m T
G: e(T*(0-5%N))

Y: 4096 v Z
W: w (E*Y*.75)+(G*C*.3)
