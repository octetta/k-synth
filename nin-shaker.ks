/ NIN Industrial Shaker/Noise Hit
/ Not a clean maraca — more like a noise burst used rhythmically
/ Very fast, very bright
N: 2205
T: !N
X: T*e(T*(0-15%N))
E: w X
/ All high content — 1-bit is perfect here
M: m T
/ Add distorted white noise on top
R: r T
L: 0.7 f R
H: R-L
Z: d((M+H)*.5*3)
W: w E*Z
