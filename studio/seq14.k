/ --- The 'Juiced' Kick (Proven) ---
B:!22050
E:(1-B%22050)^2
K:h 4*(s B*0.01)*E

/ --- The 'Firestarter' Snare (Physics Based) ---
/ 1. The 'Crack' (High Frequency Noise)
/ We filter it twice to make it 'hollow' and aggressive
N:r 11025
W:f f 5 * N * (1-!11025%11025)^8

/ 2. The 'Shell' (The 200Hz Tonal Body)
/ We use a 'square-ish' sine to make it sound like wood, not a beep
T:h 3 * (s !11025 * 0.03) * (1-!11025%11025)^4

/ 3. The Smash (Combining them)
/ We multiply the whole thing by 2 to drive it into the Tanh
S:h 2 * (W + T)

/ --- The 'Solid' Sequence ---
/ Stitching Kick, Silence, Snare, Silence
Z:0*!11025
O:K,Z,S,Z

\p O
