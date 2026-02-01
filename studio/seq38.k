/ --- 1. Timing ---
N:44100
L:!N

/ --- 2. The Sequence (2 notes to keep it simple) ---
/ We'll use 400 and 600. 
/ We skip the 'S:N#Q' for a second to ensure no overlapping.
/ We'll just make the first half 400 and the second half 600.
M:(L < 22050) * (L ! 400) + (L >= 22050) * (L ! 600)
M:(M * 0.005) - 1

/ --- 3. The Single-Path Sweep ---
/ We'll use a single variable for Cutoff to avoid 'Matrix' confusion
E:1 - (L % 22050 / 22050)
C:0.02 + (0.4 * (E * E))

/ --- 4. The Squelch (Fixed Resonance in the call) ---
/ Instead of A:C,R, we pass C and handle R as a scalar if possible
/ Or we just use a very high resonance baked into the filter call
X:C f M

/ --- 5. Final Gain ---
X:h 3 * X
\p X
