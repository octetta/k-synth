/ NIN Tom — punchy, roomy, slightly overdriven
/ Toms recorded in a room with interaction — not clean samples
/ Mid tom: ~130Hz body
N: 22050
T: !N
E: e(T*(0-6.9%N))
/ pitch sweep: more abrupt than 808
F: 130+80*e(T*(0-100%N))
D: F*(6.28318%44100)
P: +\D
O: s P

/ Soft-clip the body — the room mic compression/saturation
S: d(O*2)

/ Noise attack — stick hit + room ambience
V: e(T*(0-150%N))
R: r T
C: 0.15 f R

W: w (E*S*.8)+(V*C*.2)
