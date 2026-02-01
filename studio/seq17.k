/ --- The 'Liam' Snare (No Filter Needed) ---
/ 1. The Snap: High frequency 'grit'
R:r 4410
/ 2. The Window: A high-freq sine to 'chops' the noise into bits
/ This creates that 'crunchy' 12-bit sampler sound
W:s (!4410) * 0.8
/ 3. The Envelope: A sharp hit
E:(1-!4410%4410)^10
/ 4. Combine: Noise * Sine * Env
S:h 4 * R * W * E
/ 5. Add the 'Thump' (200Hz)
T:(s !4410 * 0.03) * (1-!4410%4410)^3
S:S + T

/ --- Sequence ---
Z:0*!11025
O:K,Z,S,Z
\p O
