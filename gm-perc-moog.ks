/ Moroder Percussion — Moog modular percussive stab
/ Short percussive hits in the Moroder arrangements (toms, accent stabs)
/ The "electronic drum" the Moog COULD do: tone burst through LP
/ "The Moog couldn't give a punch — it gave oomph"
/ This IS the oomph: short tonal burst, the Moog VCO → LP → VCA
/ C3 = 130.8Hz, 100ms
N: 4410
T: !N

F: 200+130*e(T*(0-100%N))
D: F*(6.28318%44100)
P: +\D
S: s P

/ Moog LP: moderate cutoff, decaying with pitch
C: 0.15 f S
X: 0.15 f C

E: e(T*(0-6.9%N))
W: w E*X
