/ NAP glitch stutter — bitcrushed tone with noise intrusions
N: 8820
T: !N
F: 110*(6.28318%44100)
P: +\(N#F)
S: s P
/ quantize heavily — bitcrush approximation
V: 8 v S
/ noise pops at random
R: r T
G: e(T*(0-200%N))
X: G*(R-(0.1 f R))
/ drive
E: e(T*(0-6%N))
W: w d((E*V*0.7+X*0.4)*2.0)
