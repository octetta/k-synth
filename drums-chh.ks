/ 808 Closed Hi-Hat — very bright, 200ms
/ N=8820
N: 8820
T: !N
E: e(T*(0-6.9%N))
/ m verb = 1-bit noise: bright and metallic
M: m T
/ mix 1-bit with white noise for texture
R: r T
/ HP above 3500Hz: subtract LP
L: 0.5 f R
H: R-L
W: w E*(M*.6+H*.4)
