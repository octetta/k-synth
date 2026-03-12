/ 808 Maracas — 4kHz dominant, 30ms burst
/ N=3528
N: 3528
T: !N
X: T*e(T*(0-20%N))
E: w X
R: r T
/ HP at ~3500Hz
L: 0.5 f R
H: R-L
/ 1-bit noise adds top sparkle
M: m T
W: w E*(H*.7+M*.3)
