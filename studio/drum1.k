/ drum_machine.k (v1.1.0-SYNTH Corrected)
/ 1. Setup Time and 4 Beats
T: ! 44100
R: 44100 # ! 11025
E: (1 - R % 11025) ^ 4

/ 2. THE KICK: Use R (local time) and keep phase low
/ Multiplying by 0.2 gives us the tone; E drops the pitch
K: E * s (0.2 * R * E) ! p * 2

/ 3. THE HI-HAT: Fast pattern (8 per second)
/ Pattern is a tiny pulse of high-passed noise
P: 44100 # (300 # 1) + 5212 # 0
H: P * 0.8 h r 44100 # 1

/ 4. THE MIX: 50% Kick + 20% Hat
O: (0.5 * K) + 0.2 * H

/ 5. Play it!
play O
