/ --- Setup (2756 samples per 16th note) ---
N:2756
L:!N
E:e -8 * L % N

/ --- K: Kick ---
P:0.15 * e -20 * L % N
K:E * s L * 0.05 + P

/ --- S: Snare ---
S:E * (0.5,2.0) f r N

/ --- H: Hi-Hat ---
/ Very sharp decay (-20), high cutoff (0.8)
F:e -20 * L % N
H:0.3 * F * (0.8,1.0) f r N

/ --- O: Silence (Rest) ---
O:N # 0

/ --- 16-Step Pattern: K,H,S,H,K,H,S,H ... ---
/ We'll make a classic "Four-on-the-floor" with hats
P:K,H,S,H,K,H,S,H,K,H,S,H,K,H,S,H
\p P
