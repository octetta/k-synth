/ --- The 808 Kick ---
B:!22050
K:h 3 * (s B * 0.01 * (1-B%22050)^2) * (1-B%22050)

/ --- The 'Prodigy' Snare ---
/ R: Noise burst with sharp decay
E:(1-!11025%11025)^15
R:(r 11025) * E
/ W: High-passed crack (alpha 0.4)
W:0.4 f R
/ T: Low-passed body (alpha 0.02)
T:0.02 f R
/ S: Combine, Distort, and add 500-sample slapback
S:500 y (h 5 * (W + T))

/ --- Sequence ---
Z:0*!11025
O:K,Z,S,Z
\p O
