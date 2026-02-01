N:22050
L:!N
/ Use a much quieter Sawtooth to give the filter headroom
M:((L ! 400) * 0.005) - 0.25
/ Safe Cutoff: 0.05 to 0.2
C:0.05 + 0.15 * (1 - L % N)
/ Low Resonance: 2.0 (The range is 0.0 to 4.0)
R:N # 2.0
P:C,R
X:P f M
\p X
