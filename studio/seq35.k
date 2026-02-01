/ --- 1. The Buzzy Sawtooth (Forced Length) ---
L:!44100
/ Wrap every 400 samples to get a 110Hz buzz
M:L ! 400
/ Normalize 0..400 to -1..1
M:(M * 0.005) - 1

/ --- 2. The Filter Sweep ---
/ We want it to start OPEN (bright) and slam SHUT (muffled)
/ We use L again so we don't have to re-calculate !44100
E:1 - (L % 44100)
/ C: Cutoff (0.5 down to 0.02)
C:0.02 + (0.48 * (E * E))
/ R: Resonance (High squelch)
R:3.8

/ --- 3. The Synthesis ---
/ Combine Cutoff and Resonance
A:C,R
X:A f M

/ --- 4. Final Polish ---
X:h 3 * X
\p X
