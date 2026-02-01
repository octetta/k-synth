/ drum_machine.k
/ 1. Setup Time and Beats (4 per second)
T: ! 44100
R: 44100 # ! 11025
E: (1 - R % 11025) ^ 4

/ 2. THE KICK: Pitch-dropped Sine
/ (0.1 * E) makes the pitch drop as the sound fades
K: E * s T * 0.1 * E

/ 3. THE HI-HAT: High-passed Noise
/ Pattern: Hit on every 8th note (8 per second)
P: 44100 # (500 # 1) + 5012 # 0
H: P * 0.7 h r 44100 # 1

/ 4. THE MIX: Kick + Hat
/ We scale them so they don't clip (distort)
O: (0.5 * K) + 0.3 * H

/ 5. Play it!
play O
