\l setup.k

/ Constant 90 Hz tone
R: 44100
T: 90 % R
B: s p T * A

/ Very steep envelope for punch
K: 0.9 & -0.9 | 6 * B * E ^ 4

