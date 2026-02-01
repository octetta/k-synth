/ --- The 808 Kick (The Standard) ---
B:!22050
K:h 4 * (s B * 0.01 * (1-B%22050)^2) * (1-B%22050)

/ --- The 'Prodigy' Snare (The Beast) ---
/ 1. The Snap: High-passed noise (Bright and Crunchy)
/ We use a high alpha (0.4) so it's not 'muffled'
R:r 11025
W:0.4 f R * (1-!11025%11025)^10

/ 2. The Shell: Low-passed resonance (The 'Wood' sound)
/ We use a tiny alpha (0.02) to turn noise into a 'thud'
T:0.02 f (r 11025) * (1-!11025%11025)^5

/ 3. The Crack: A 1kHz tonal spike
C:s (!11025) * 0.15 * (1-!11025%11025)^25

/ Mix: Smash them all into the Tanh
/ This is where the 'white noise blast' becomes a snare
S:h 3 * (W + T + C)

/ --- The Sequence ---
Z:0*!11025
O:K,Z,S,Z
\p O
