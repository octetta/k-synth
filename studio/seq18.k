/ --- The 808 Kick (The Baseline) ---
B:!22050
K:h 4 * (s B * 0.01 * (1-B%22050)^2) * (1-B%22050)

/ --- The 'Bit-Crushed' Snare ---
/ 1. The Noise
R:r 4410
/ 2. The 'Crush': Multiply by a high-freq sine to 'shred' the noise
/ This acts as a 'pseudo-filter' that adds grit
C:s (!4410) * 0.5
/ 3. The Envelope: Extreme decay
E:(1-!4410%4410)^15
/ 4. The 'Ring': A 300Hz wooden knock
T:(s !4410 * 0.04) * (1-!4410%4410)^5
/ 5. Combine and OVERDRIVE
S:h 6 * (R * C * E + T)

/ --- The Sequence ---
Z:0*!11025
O:K,Z,S,Z
\p O
