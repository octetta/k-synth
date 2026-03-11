/ Closed Hi-Hat
/ 100ms at 44100 = 4410 samples

N: 4410
T: !N
E: e(T*(0-6.9%N))
R: r T
M: m T
/ highpass: subtract lowpass from original
L: 0.15 f R
H: R-L
S: H*.7+M*.3
W: w E*S
