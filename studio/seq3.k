/ --- The 'Heavy' Kick (BOOM) ---
B:!11025
/ L: A much slower decay for the sub-bass
E:(1-B%11025)^2
/ Pitch: Starts at 150Hz and drops to 40Hz
P:s B * 0.1 * (0.05 + E)
/ K: Saturate the kick with 'h' (Tanh) for that thick 'oomph'
K:h 2 * E * P

/ --- The 'Aggressive' Snare (SNAP) ---
R:r 11025
/ Noise decay is now linear (-) joined with a sharp exponential (^) 
/ to ensure the 'crack' stays loud
S:h 3 * R * (1-B%11025)^3

/ --- The Mix ---
T:!44100
J:_T%11025
/ Create the sequence and BOOST the whole thing
O:2 * ((J!2)=0 * 44100#K) + ((J!2)=1 * 44100#S)
/ Final Soft-Clip to prevent digital nastiness
O:h O

\p O
