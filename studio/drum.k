/ 1. Setup 4 beats
R: 44100 # ! 11025

/ 2. Create the Decay Envelope
E: (1 - R % 11025) ^ 4

/ 3. Pitch-modulated Sine: (300 * E) makes the pitch slide 
/ We multiply by R to move through the sine wave
K: s R * 0.1 * E

/ 4. Final mix: apply volume envelope to the pitch-dropped sine
O: K * E

play O
