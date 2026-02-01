/ --- 1. The Buzzy Sawtooth ---
/ We'll generate a ramp and use ! to wrap it every 100 samples
/ This creates a ~441Hz buzz (A4)
M:(!44100) ! 100
/ Normalize it to -1.0 to 1.0 range
M:(M * 0.02) - 1

/ --- 2. The Filter 'Movement' ---
/ If the tone was steady, the filter wasn't sweeping.
/ Let's force a massive, slow sweep over the whole second.
B:!44100
/ E goes from 1.0 down to 0.0
E:1 - (B % 44100 % 44100)
/ C (Cutoff) goes from 0.4 down to 0.01
C:0.01 + (0.39 * E)
/ R (Resonance) - Let's set it to 3.8 (near whistling)
R:3.8

/ --- 3. The Squelch ---
/ Apply the resonant filter using our (C,R) matrix
X:(C,R) f M

/ --- 4. Final Smash ---
X:h 3 * X
\p X
