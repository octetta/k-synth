/ --- The 'Prodigy' Drum Kit ---
/ Kick (K)
B:!22050
K:h 4 * (s B * 0.01) * (1 - (B % 22050))

/ Snare (S) - Using our parenthesized 'Socks-Off' logic
N:r 11025
E:(1 - ((!11025) % 11025)) ^ 12
S:h 5 * (0.3 f N) * E

/ --- The 'Ghost' Logic ---
/ Loud Snare (L) and Ghost Snare (G)
L:S
G:0.3 * S

/ --- The 8-Step Breakbeat ---
/ Sequence: Kick, Ghost, Snare, Ghost, Kick, Kick, Snare, Silence
/ Each 'slot' is 11025 samples (0.25s)
Z:0 * !11025
O:K,G,L,G,K,K,L,Z

\p O
