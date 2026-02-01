/ --- The 808 Kick ---
B:!22050
K:h 4 * (s B * 0.01 * (1-B%22050)^2) * (1-B%22050)

/ --- The 'Firestarter' Snare (Enhanced) ---
/ 1. The Snap
R:r 11025
W:0.5 f R * (1-!11025%11025)^10
/ 2. The Body (Lower freq resonance)
T:0.03 f (r 11025) * (1-!11025%11025)^4
/ 3. The Smash
S:h 3 * (W + T)
/ 4. THE ECHO: 1000 sample delay (approx 22ms)
/ This creates a 'slapback' that makes the snare sound HUGE
S:1000 y S

/ --- The Sequence ---
Z:0*!11025
O:K,Z,S,Z
\p O
