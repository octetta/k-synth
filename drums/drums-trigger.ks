/ 808 Trigger Out — low thump ~100Hz, 650ms
/ N=28665
N: 28665
T: !N
E: e(T*(0-6.9%N))
/ no pitch sweep, fixed ~100Hz
F: 100*((p 2)%(p 0))
P: +\(N#F)
S: (s P)
/ brief sub attack
Q: e(T*(0-150%N))
R: r T
C: 0.04 f R
W: w (E*S)+(Q*C*.08)
