/ --- 1. Define Length & Ramp ---
N:44100
L:!N

/ --- 2. The Oscillator (Forced Sequence) ---
/ We must ensure the 'Mod' happens to the whole array L
M:L ! 400
/ Center and scale the buzz
M:(M * 0.005) - 1

/ --- 3. The Filter Envelope ---
/ We use L again to ensure E is 44100 long
E:1 - (L % N)
/ Squaring E makes the sweep 'snap' faster
C:0.02 + (0.5 * (E * E))
R:3.8

/ --- 4. The Squelch ---
/ We join C and R, then apply to M
A:C,R
X:A f M

/ --- 5. Mastering ---
X:h 3 * X
\p X
