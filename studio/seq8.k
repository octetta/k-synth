/ --- The 'Juiced' 808 Kick ---
B:!22050
/ E: A long, fat linear decay for the sub-bass body
E:(1-B%22050)
/ P: The Pitch Drop. Starts high and slams into the sub zone (0.01)
P:s B * (0.07 * E^3 + 0.01)
/ K: The Juice. We multiply by 4 then Tanh to 'square' the wave.
/ This creates that heart-thumping harmonic 'growl'.
K:h 4 * P * E^2

/ --- The 'Prodigy' Snare (The Concrete Smasher) ---
/ R: Pure high-density noise
R:r 11025
/ E2: A very 'gated' envelope. Stays loud, then drops instantly.
E2:(1-B%11025)^2
/ T: A mid-range 'ring' at 400Hz to give it that 'wooden' crack
T:s B * 0.6 * (1-B%11025)^15
/ S: Smash the noise and the ring together and OVERDRIVE it
S:h 6 * (R + T) * E2

/ --- The Master Sequencer ---
T:!44100
J:_T%11025
/ M: Kick on 1 and 3
M:(J!2)=0
/ N: Snare on 2 and 4
N:(J!2)=1
/ Final Mix: We boost the whole bus and clip it again
O:h 1.5 * ((M * 44100#K) + (N * 44100#S))

\p O
\w 1000
\p O
