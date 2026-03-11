/ Moroder Sub Bass — Moog Taurus pedal synth / Moog modular sub VCO
/ The Moog Taurus was in his collection; also the Moog modular could do sub-octave
/ Used to reinforce the sequenced bass with a sub octave layer
/ Pure sine-like sub — the Moog ladder LP so tight only the fundamental passes
/ C1 = 32.7Hz, one octave below the IFL bassline root
N: 8820
T: !N

F: 32.7*(6.28318%44100)
P: +\(N#F)
S: s P

/ Moog ladder LP so tight only sine survives (ct=0.01 → ~70Hz)
C: 0.01 f S

E: e(T*(0-6%N))
W: w E*C
