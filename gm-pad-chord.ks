/ Moroder Chord Pad — Moog modular / Polymoog, sustained chord pads
/ "Some of the chords—which were difficult to do because I had to do
/  each note of a chord separate on different tracks" — Moroder
/ The IFL sustained chord texture: each note on its own Moog track
/ Warm, slightly nasal Moog character — the LP near self-oscillation
/ This voice = one sustained chord note (layer 4 for full chord)
/ C4 = 261.6Hz, 1000ms
N: 44100
T: !N

/ Single Moog VCO (one note per track as Moroder described)
F: 261.6*(6.28318%44100)
G: 262.4*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)

/ Sawtooth VCO
A: 1 .5 .333 .25 .2 .167 .143
S: P $ A
U: Q $ A
V: (S+U)*.5

/ Moog LP at moderate cutoff — the "warmth" zone
/ Not wide open (that would be too bright), not too dark
/ ct=0.15 → ~1053Hz — right in the warm nasal zone
C: 0.15 f V
X: 0.15 f C

/ Slow attack, long sustain (sustained chord pad)
E: e(T*(0-3%N))

W: w E*X
