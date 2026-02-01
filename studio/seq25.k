/ --- The High-Hat (H) ---
N:r 5512
/ High-pass trick: Noise minus its low-passed version (0.1 cutoff)
H:N - (0.1 f N)
/ Sharp envelope for a 'chick' sound
E:(1 - ((!5512)%5512))^15
H:H * E

/ --- The Bass & Kick (Recap) ---
B:!11025
K:h 3 * (s B * 0.01) * (1 - (B % 11025))
D:1 - (a K)
M:(s B * 0.008) * D

/ --- The Snare (S) ---
E:(1 - ((!11025)%11025))^12
S:h 4 * (0.3 f (r 11025)) * E

/ --- The Full Pattern ---
/ We'll weave the hat (H) into the gaps
/ Slot size is 11025, but Hat is 5512, so we double the Hat
P:H,H
O:(K+M),P,(S+M),P,K,(K+M),S,P
\p O
