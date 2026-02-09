\l setup.k
/ Body: pitch sweep using chirp
B: s 0.006 * A

/ Click: brief high frequency
C: s 0.08 * A

/ Layer with different envelopes
K: 0.7 & -0.7 | (4 * B * E * E) + (0.4 * C * P * P)

