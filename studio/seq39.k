/ --- 1. Basic Setup ---
N:22050
L:!N

/ --- 2. Note A (Low Buzz) ---
/ 400 sample reset = ~110Hz
M:L ! 400
M:(M * 0.005) - 1
/ Filter for Note A
E:1 - (L % N)
C:0.02 + (0.4 * (E * E))
A:C f M

/ --- 3. Note B (High Buzz) ---
/ 200 sample reset = ~220Hz
M:L ! 200
M:(M * 0.01) - 1
/ Filter for Note B
E:1 - (L % N)
C:0.02 + (0.6 * (E * E))
B:C f M

/ --- 4. Join & Master ---
/ Join the two 0.5s chunks into a 1s track
X:A,B
X:h 3 * X
\p X
