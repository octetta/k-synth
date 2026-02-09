\l setup.k

/ Two detuned oscillators for "buzz"
/ ~160 Hz
R: 44100
X: 160 % R
T: s p X * A
/ ~170 Hz (slightly sharp)
Y: 170 % R
U: s p Y * A

/ Noise component (heavy filtering)
N: 0.65 f r A

/ Mix: tonal body + noise snap
S: 0.8 & -0.8 | (2 * (T + U) * M) + (1.2 * N * M)

