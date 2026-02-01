/ 1. Setup 1 second of noise
N: r 44100 # 1

/ 2. Filter the noise (High-pass to remove rumble)
/ 0.6 h N gives it that 'crispy' snare wire sound
F: 0.6 h N

/ 3. Create a longer decay (1/4 second hits)
R: 44100 # ! 11025
E: (1 - R % 11025) ^ 10

/ 4. Apply the envelope to the filtered noise
S: F * E

play S
