/ --- Setup ---
N:5512
L:!N
E:e -5 * L % N

/ --- K: The Kick ---
/ Rapid pitch drop sine wave
P:0.1 * e -15 * L % N
K:E * s L * 0.05 + P

/ --- S: The Snare ---
/ Filtered noise (r) with a quick decay
/ We use a high-pass-style filter (C:0.4) to remove the mud
R:N # 1.5
S:E * (0.4,R) f r N

/ --- The Pattern ---
/ K, S, K, S
Z:K,S,K,S
\p Z
