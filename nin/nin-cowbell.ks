/ NIN Industrial Cowbell — ring-modulated, harsh
/ Less musical than 808 cowbell — treated as a noise instrument
N: 22050
T: !N
E: e(T*(0-6.9%N))

/ Two tones — 562Hz and 845Hz (classic ratios) but heavily processed
F: 562*((p 2)%(p 0))
G: 845*((p 2)%(p 0))
P: +\(N#F)
Q: +\(N#G)
O: s P
I: s Q

A: 1 0 0.3 0 0.15
J: P $ A
K: Q $ A
M: J+K*.8

/ Ring mod carrier to make it industrial/harsh
L: 280*((p 2)%(p 0))
R: +\(N#L)
C: s R
Y: M*C

/ Clip it
D: d(Y*3)

W: w E*D
