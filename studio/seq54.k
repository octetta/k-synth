N:22050
L:!N
M:((L ! 400) * 0.01) - 0.5
/ Sweep cutoff 0.1 to 0.4
C:0.1 + 0.3 * (1 - L % N)
/ Keep resonance at a modest 2.5
R:N # 2.5
P:C,R
X:P f M
\p h 4 * X
