/ 1. 4 Beats
R: 44100 # ! 11025
/ 2. Sharp decay (Power of 8)
E: (1 - R % 11025) ^ 8
/ 3. Solid 60Hz tone (approx)
/ Dividing by 100 is the magic number for bass
K: E * s R % 100
play K
