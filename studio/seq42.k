/ --- 1. The Raw Oscillator (Force 1.0 amplitude) ---
L:!5512
/ We'll use 400 for a deep bass note. 
/ (L!400)/200-1 ensures the wave spans from -1.0 to 1.0
M:( (L ! 400) * 0.005 ) - 1

/ --- 2. The 'Wide Open' Filter ---
/ If it's clicking, the filter is too low. 
/ Let's move the range to 0.1 (low) and 0.8 (high)
E:1 - (L / 5512)
C:0.1 + (0.7 * E)
A:C f M

/ --- 3. Higher Note ---
M:( (L ! 200) * 0.01 ) - 1
C:0.1 + (0.8 * E)
B:C f M

/ --- 4. Assemble & Saturate ---
X:A,B,B,A
/ h (tanh) will 'glue' these into a growl
X:h 2 * X
\p X
