/ 1s of noise
N:r 44100
/ Cutoff ramp (C) and high Resonance (R)
C:0.01 + (0.3 * (!44100) % 44100)
R:3.5
/ Join them to pass as a single argument to 'f'
A:C,R
/ Apply the resonant filter
X:A f N
\p X
