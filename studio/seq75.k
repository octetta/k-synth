/ --- The Single Kick (1/4 second) ---
N:5512
L:!N
/ P: Pitch Envelope. Starts high, drops fast.
P:0.1 * e -10 * L % N
/ M: The Sine Oscillator. Pitch is modulated by P.
/ We multiply L by (0.05 + P) to create the 'thump'.
M:s L * 0.05 + P

/ --- The Amplitude Envelope ---
/ E: A sharp decay so the drum doesn't click at the end.
E:e -6 * L % N
K:E * M

/ --- The Rhythm ---
/ We want 4 hits. Total length 22050.
/ Use Reshape (#) to repeat the 5512-sample 'K' array.
Z:22050 # K
\p Z
