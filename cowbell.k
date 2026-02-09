\l setup.k

R: 44100

/ ~900 Hz
X: 900 % R
T: s p X * A

/ ~1350 Hz
Y: 1350 % R
U: s p Y * A

/ Sharp attack, medium decay
C: 0.7 & -0.7 | (T + U * 0.8) * M

