\l setup.k

/ Two metallic frequencies (inharmonic)
/ ~300 Hz
S:44100
X:300%F
Y:440%F
T: s p X * A
/ ~440 Hz
U: s p Y * A

/ Very sharp envelope
R: 0.7 & -0.7 | 3 * (T + U * 0.6) * P * P

