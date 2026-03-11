/ Rimshot — short bright crack
/ 120ms at 44100 = 5292 samples

N: 5292
T: !N
R: r T
/ keep high end: subtract lowpass at ~3500Hz
B: 0.5 f R
H: R-B
/ mid layer: subtract lowpass at ~1000Hz
J: r T
M: 0.15 f J
K: J-M
E: e(T*(0-6.9%N))
/ bright crack dominant, mid layer underneath
W: w E*(H+(K*.5))
