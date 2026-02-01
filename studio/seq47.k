/ --- 1. Master Timing ---
N:22050
L:!N

/ --- 2. The Acid Line ---
/ We force the sawtooth to be -1.0 to 1.0
M:( (L ! 400) * 0.005 ) - 1
C:0.05 + 0.6 * (1 - (L % 5512 / 5512)) ^ 2
X:C f M
/ Re-center X just in case the filter shifted it
X:X - 0.5

/ --- 3. The Sidechain Pump ---
G:(L % 5512) / 5512
X:X * G

/ --- 4. The Kick Drum ---
/ We'll use a Sine wave (which is naturally centered)
/ but we must ensure the volume envelope doesn't kill it
K:s (L % 5512) * 0.05
/ Shape the kick volume
K:K * (1 - (L % 5512 / 5512)) ^ 4

/ --- 5. The Final Master ---
/ If the signal is too quiet, we'll boost it here
Z:X + K
Z:h 5 * Z
\p Z
