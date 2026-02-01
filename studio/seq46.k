/ --- 1. Master Timing ---
N:22050
L:!N
R:3.8

/ --- 2. The Raw Acid Line (No FX yet) ---
/ We'll generate a single 1s buzzing pattern
M:(L ! 400) * 0.005 - 1
/ Filter sweep that repeats every 5512 samples
C:0.05 + 0.6 * (1 - (L % 5512 / 5512)) ^ 2
X:C f M

/ --- 3. The Sidechain (The 'Pump') ---
/ Create a volume ramp G that is exactly N samples long
G:(L % 5512) / 5512
/ Apply ducking: Multiply Acid by the ramp
X:X * G

/ --- 4. The Kick Drum (The 'Thump') ---
/ Simple sine drop
K:s (L % 5512) * 0.01
/ Shape it into a percussive hit
K:K * (1 - (L % 5512 / 5512)) ^ 4
/ Ensure K is exactly N long (it already is because it uses L)

/ --- 5. The Final Master ---
/ Add them together and saturate
Z:X + K
Z:h 4 * Z
\p Z
