/ --- The 'Acid' Setup ---
B:!44100
/ Sawtooth-ish wave (The 'Raw' Sound)
M:(B % 200) % 100
M:M * 0.01

/ --- The Squelchy Sweep ---
/ Cutoff (C) ramps from 0.05 up to 0.4
C:0.05 + (0.35 * (!44100) % 44100)
/ Resonance (R) is a fixed high value (0.8)
R:0.8
/ Join them to create a 2-column automation matrix
A:C,R

/ Apply the Resonant Filter
M:A f M

/ Final Drive
M:h 4 * M
\p M
