\l setup.k

/ Tonal part with slight pitch drop
T: s p 0.004 * A * (1 - 0.0001 * A)

/ Resonant filtered noise for "snap"
N: (0.4;2.8) f r A

/ Combine
S: 0.85 & -0.85 | (1.8 * T * M) + (1.5 * N * P)

