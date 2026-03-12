/ Initialize
A: !12000
E: 1 - 0.00008 * A
P: E ^ 6
M: E ^ 3
Q: E ^ 2

/ Extreme pitch sweep down
D: (1 - 0.0005 * A) ^ 2
L: s p (0.05 * D) * A
Z: L * M * 2

\p Z
