/ 808-style Bass Drum
/ 800ms at 44100 = 35280 samples

N: 35280
T: !N
E: e(T*(0-6.9%N))
/ pitch sweep 160->50 Hz
F: 50+110*e(T*(0-140%N))
D: F*(6.28318%44100)
P: +\D
S: s P
/ very short body thump — heavily lowpassed noise at attack
Q: e(T*(0-220%N))
R: r T
C: 0.04 f R
W: w (E*S)+(Q*C*.15)
