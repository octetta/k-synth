/ NIN Rimshot — industrial crack, very fast, harsh
/ Less tonal than 808 rim — more like a metal rod hit
/ 80ms
N: 3528
T: !N
E: e(T*(0-6.9%N))

/ Two high tones — 1800Hz + 2400Hz
F: 1800*(6.28318%44100)
G: 2400*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
O: s P
I: s Q
S: (O+I*.6)

/ Ring mod on the tonal mix — extra metallic
K: 520*(6.28318%44100)
J: +\(N#K)
M: s J
Y: S*M

/ Clip it hard
D: d(Y*5)

/ Fast broadband noise burst — the stick crack
V: e(T*(0-200%N))
R: r T
L: 0.5 f R
H: R-L

W: w (D*E*.7)+(H*V*.5)
