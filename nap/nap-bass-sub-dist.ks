/ NAP sub distortion bass — pure sub driven into saturation
/ the harmonic distortion creates the grit
N: 13230
T: !N
F: 41.2*(6.28318%44100)
P: +\(N#F)
S: s P
/ soft clip then hard clip stack
B: d(S*2.0)
C: d(B*3.0)
/ LP to focus the sub weight
L: 0.08 f C
E: e(T*(0-4%N))
W: w E*L
