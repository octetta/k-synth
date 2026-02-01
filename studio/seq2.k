/ --- The 'Boom' Kick ---
B:!11025
/ E: Softer decay (^3 instead of ^5) for more 'thump'
E:(1-B%11025)^3
K:0.8 * E * s B * 0.08 * (0.1 + E^2)

/ --- The 'Snap' Snare ---
R:r 11025
/ E2: Sustained noise (^5 instead of ^10)
E2:(1-B%11025)^5
/ Adding a tiny bit of low-end 'thwack' (T) to the noise (R)
T:s B * 0.2 * (1-B%11025)^20
S:(R * E2 * 0.6) + T

/ --- Sequencer ---
T:!44100
J:_T%11025
O:((J!2)=0 * 44100#K) + ((J!2)=1 * 44100#S)

/ Play and Repeat
\p O
\w 1000
\p O
