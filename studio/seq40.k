/ --- 1. The Note Factory Setup ---
/ L is our 1/8th note duration (5512 samples at 44.1k)
L:!5512
/ R is our fixed Resonance for the whole session
R:3.5

/ --- 2. Step 1: Low C ---
M:(L ! 400) * 0.005 - 1
C:0.01 + 0.4 * (1 - (L % 5512 / 5512)) ^ 4
X:C f M

/ --- 3. Step 2: Mid G ---
M:(L ! 300) * 0.005 - 1
C:0.01 + 0.5 * (1 - (L % 5512 / 5512)) ^ 4
X:X , (C f M)

/ --- 4. Step 3: High C ---
M:(L ! 200) * 0.005 - 1
C:0.01 + 0.6 * (1 - (L % 5512 / 5512)) ^ 4
X:X , (C f M)

/ --- 5. Step 4: Back to Low ---
M:(L ! 400) * 0.005 - 1
C:0.01 + 0.4 * (1 - (L % 5512 / 5512)) ^ 4
X:X , (C f M)

/ --- 6. Final Drive ---
X:h 3 * X
\p X
