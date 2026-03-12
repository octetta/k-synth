/ Setup
A: !12000
E: 1 - 0.00008 * A
P: E ^ 6
M: E ^ 3

/ === KICK ===
/ Pitch sweep method
T: 1 - 0.0008 * A
T: T ^ 3
B: s p (0.00272 + (0.00454 * T)) * A
K: 0.8 & -0.8 | 5 * B * M * M

/ === SNARE ===
T: s p 0.0036 * A
U: s p 0.00385 * A
N: 0.65 f r A
S: 0.8 & -0.8 | (2 * (T + U) * M) + (1.2 * N * M)

/ === CLOSED HAT ===
H: (0.98 f r A) * P * 0.6

/ === OPEN HAT ===
O: (0.94 f r A) * E * 0.7

/ === LOW TOM ===
B: s p 0.00363 * A
C: s p 0.00726 * A * 0.3
N: 0.75 f r A * P
D: 0.75 & -0.75 | (3 * (B + C) * M) + (0.5 * N)

/ === HIGH TOM ===
B: s p 0.0068 * A
C: s p 0.0136 * A * 0.3
N: 0.75 f r A * P
F: 0.75 & -0.75 | (3 * (B + C) * M) + (0.5 * N)

/ === CLAP ===
N: 0.55 f r A
L: s 45 * A
C: N * (P + (0.25 * P * L) + (0.15 * P * P))

/ === RIM ===
T: s p 0.0068 * A
U: s p 0.01 * A
R: 0.7 & -0.7 | 3 * (T + U * 0.6) * P * P

/ === COWBELL ===
T: s p 0.0204 * A
U: s p 0.0306 * A
B: 0.7 & -0.7 | (T + U * 0.8) * M

/ Play them
\p K
\p S
\p H
\p O
\p D
\p F
\p C
\p R
\p B

