/ --- 1. Timing ---
L:!44100
/ --- 2. The Oscillator (The buzz we know works) ---
M:((L ! 400) * 0.01) - 0.5

/ --- 3. The Filter (Cutoff Only) ---
/ We'll make it sweep from 0.8 down to 0.05
C:0.05 + 0.75 * (1 - (L / 44100))
/ Pass ONLY C. Let's see if the engine likes this better.
X:C f M

/ --- 4. Boost and Play ---
Z:h 4 * X
\p Z
