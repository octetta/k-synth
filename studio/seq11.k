/ --- The 'Juiced' Kick (Keep this, it's working!) ---
B:!22050
E:1-B%22050
P:s B*0.01*E
K:h 3*P*E

/ --- The 'Prodigy' Snare (The Layered Smash) ---
/ 1. The Snap: Raw noise with a very sharp decay
R:r 11025
W:R*(1-!11025%11025)^5
/ 2. The Body: A 200Hz 'knock' that rings slightly longer
/ We use a smaller multiplier (0.03) to get that woody resonance
T:s (!11025)*0.03*(1-!11025%11025)^3
/ 3. The Smash: Combine them and DRIVE it into the Tanh
/ The 0.7 vs 0.5 balance ensures the 'thwack' is heard through the hiss
S:h 4*(0.7*W+0.5*T)

/ --- The Sequence (Stitched for Memory Stability) ---
O:K,S,K,S
\p O
