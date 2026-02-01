/ --- THE JUICE (808) ---
B:!22050
/ Pitch: Slow glide from 60Hz down
P:s B*0.01*(1-B%22050)
/ The Kick (0.5s)
K:h 3*P*(1-B%22050)

/ --- THE SMASH (SNARE) ---
R:r 22050
/ The Snare (0.5s) - Gated Noise
S:h 4*R*(1-!22050%22050)^2

/ --- THE SEQUENCE ---
/ Join them: Kick , Snare , Kick , Snare (2.0 seconds total)
O:K,S,K,S

\p O
