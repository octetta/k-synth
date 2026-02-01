/ --- 1. Timing & Note Base ---
L:!5512
/ High Resonance: 3.8 makes it whistle.
R:3.8

/ --- 2. Note A (Low Growl) ---
M:((L ! 400) * 0.005) - 1
/ Filter: Start very bright (0.9) and drop fast
E:1 - (L / 5512)
C:0.05 + (0.85 * E * E)
/ Pass (C,R) to get the resonance whistle
A:(C,R) f M

/ --- 3. Note B (High Squelch) ---
M:((L ! 250) * 0.008) - 1
C:0.05 + (0.9 * E * E)
B:(C,R) f M

/ --- 4. Note C (The Peak) ---
M:((L ! 150) * 0.012) - 1
C:0.1 + (0.8 * E * E)
D:(C,R) f M

/ --- 5. Assemble & Heavy Drive ---
X:A,B,D,B
/ Drive it hard (8x gain) into the tanh 'h' for distortion
X:h 8 * X
\p X
