N:44100
L:!N
/ Oscillator: Sawtooth
M:((L ! 200) * 0.01) - 0.5
/ Cutoff: High to low
C:0.01 + 0.4 * (1 - L % N)
/ Resonance: Fixed high
R:N # 3.8
/ Join and Filter
X:(C,R) f M
\p h 4 * X
