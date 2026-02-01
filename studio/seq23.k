/ --- The Core Kit ---
B:!11025
K:h 4 * (s B * 0.01) * (1 - (B % 11025))
N:r 11025
E:(1 - ((!11025) % 11025)) ^ 12
S:h 5 * (0.3 f N) * E

/ --- The Randomizer (Using single letters) ---
/ G, H, and I are our three unique ghost hits
G:(a r 1) * S
H:(a r 1) * S
I:(a r 1) * S

/ --- The Sequence ---
/ K: Kick, S: Snare, G/H/I: Random Ghosts, Z: Silence
Z:0 * !11025
O:K,G,S,H,K,K,S,I
\p O
