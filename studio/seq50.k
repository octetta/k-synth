/ --- 1. Setup ---
N:44100
L:!N
M:((L ! 400) * 0.01) - 0.5

/ --- 2. The Parameter Stack ---
/ C is our cutoff sweep
C:0.02 + 0.4 * (1 - L/N) ^ 2
/ R is our resonance (we want it to increase as the note decays)
R:2.0 + 1.8 * (L/N)

/ --- 3. Join them (C followed by R) ---
/ This creates an array of length 88200
P:C,R

/ --- 4. Filter ---
/ The 'Smart Filter' sees P is 2x longer than M and splits it
X:P f M
X:h 6 * X
\p X
