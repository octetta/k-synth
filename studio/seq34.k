/ --- 1. The Buzzy Sawtooth ---
/ We need a low-pitched buzz (approx 110Hz). 44100/110 = 400.
M:(!44100) ! 400
/ Normalize 0-400 to -1.0 to 1.0
M:(M * 0.005) - 1

/ --- 2. The 'Acid' Decay ---
/ We want the filter to start OPEN and slam SHUT.
/ If your previous filter increased, your ramp was backwards.
B:!44100
/ This creates 1.0 down to 0.0
E:1 - (B % 44100 % 44100)
/ We'll make the sweep MORE aggressive: 0.6 down to 0.01
C:0.01 + (0.59 * (E * E))

/ --- 3. The Squelch (Max Resonance) ---
/ We'll push resonance to 3.9 (just below the 'scream' point)
R:3.9
A:C,R
X:A f M

/ --- 4. The Tube Amp ---
/ Use h (tanh) to beef up the low end
X:h 5 * X
\p X
