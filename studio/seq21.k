/ --- The 'Fat' Snare ---
/ 1. The Hiss: High-passed noise that dies fast
N:r 11025
E:(1 - ((!11025)%11025))^12
H:0.5 f (N * E)

/ 2. The Thump: Low-passed noise that lingers a bit
/ We use a lower power (^6) so the body lasts longer than the crack
B:(1 - ((!11025)%11025))^6
T:0.05 f (N * B)

/ 3. The Smash: Combine and drive into the Tanh
/ We add a little 1000Hz tone (C) for 'stick' sound
C:(s (!11025) * 0.15) * E
S:h 5 * (H + T + (0.5 * C))

/ --- The Industrial Beat ---
K:h 4 * (s (!22050) * 0.01) * (1 - ((!22050)%22050))^2
Z:0 * !11025
O:K,Z,S,Z
\p O
