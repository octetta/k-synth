/ --- New Setup (Slower Tempo) ---
N:5512
L:!N
/ E: Universal Decay Envelope
E:e -6 * L % N

/ --- K: Deeper Kick ---
/ Lower base frequency (0.02) and a slower pitch drop (-10)
P:0.1 * e -10 * L % N
K:E * s L * 0.02 + P

/ --- S: Snare ---
/ Lowering the cutoff (0.3) makes the snare 'thicker'
S:E * (0.3,1.5) f r N

/ --- H: Hi-Hat ---
/ Keep it short, but lower the volume (0.2)
F:e -25 * L % N
H:0.2 * F * (0.7,1.0) f r N

/ --- The Pattern (K H S H) ---
/ This is now 4 steps per second
Z:K,H,S,H,K,H,S,H
\p Z
