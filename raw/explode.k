\l setup.k

/ Low rumble with noise burst
R: 44100
/ ~50 Hz
B: s p (50 % R) * A
N: 0.4 f r A

/ Pitch drops, noise fades
D: 1 - 0.0002 * A
X: (2 * B * (D ^ 2)) + (N * P)

