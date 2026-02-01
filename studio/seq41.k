/ --- 1. Timing ---
L:!5512
R:3.8

/ --- 2. Note 1 (Low but audible) ---
M:(L ! 400) * 0.01 - 0.5
/ We raise the floor from 0.01 to 0.05 so it's not a 'click'
C:0.05 + 0.3 * (1 - (L % 5512 / 5512)) ^ 2
A:C f M

/ --- 3. Note 2 (Mid - The 'Beep') ---
M:(L ! 300) * 0.01 - 0.5
C:0.05 + 0.5 * (1 - (L % 5512 / 5512)) ^ 2
B:C f M

/ --- 4. Note 3 (High - The 'Squelch') ---
M:(L ! 200) * 0.01 - 0.5
C:0.05 + 0.7 * (1 - (L % 5512 / 5512)) ^ 2
D:C f M

/ --- 5. Assemble the Sequence ---
/ Pattern: Low, Mid, High, Low
X:A,B,D,A
X:h 4 * X
\p X
