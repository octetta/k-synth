/ 909 set version 001
A: !8000
E: 1 - 0.000125 * A

/ Kick
K: 0.5 & -0.5 | 5 * (s 0.007 * A) * E * E

N: 0.5 f r A
T: (s 0.2 * P * A) * P

/ Snare
S: 0.7 & -0.7 | (2 * T) + N * E * E

/ closed Hi-hat
P: E * E * E * E * E
H: (0.95 f r A) * P

/ Open hi-hat 
O: (0.95 f r A) * (0.8 * E) + (0.5 * P)

L: (s 0.5 * A) & (s 0.6 * A) | (s 0.7 * A)

/ broken clap
X: (0.5 f r A) * (P + (0.2 * L * P))
