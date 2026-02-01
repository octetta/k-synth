/ --- 1. The Oscillator (Forced Evaluation) ---
/ We wrap the Iota in parens so we Mod the whole 1s ramp
M:(!44100) ! 100
/ Normalize 0..100 to -1..1
M:(M * 0.02) - 1

/ --- 2. The Filter (Forced Evaluation) ---
/ We need a ramp that lasts the whole length
B:!44100
/ Envelope: Force the decay math
E:1 - (B % 44100)
/ Cutoff: High to low sweep
C:0.01 + (0.4 * E)
R:3.5

/ --- 3. The Squelch ---
/ Apply filter. Ensure C is the first arg.
X:(C,R) f M

/ --- 4. The Drive ---
X:h 4 * X
\p X
