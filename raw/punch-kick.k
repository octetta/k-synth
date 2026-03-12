\l setup.k

/ Low fundamental ~55 Hz
B: s p 0.0025 * A

/ Harmonic overtone for attack
H: s p 0.005 * A

/ Mix: sharp attack, sustained body
K: 0.75 & -0.75 | (3.5 * B * M) + (1.5 * H * P)

