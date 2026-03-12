/ Moroder Lead — Moog modular / Minimoog, "The Chase", "From Here to Eternity"
/ The melodic leads: Moog modular sawtooth → Moog LP with envelope filter sweep
/ Moroder's leads had a distinctive filter sweep opening: dark attack → bright peak
/ The LP cutoff controlled by envelope: starts low, opens up, then closes
/ Single VCO sawtooth (or two VCOs slightly detuned for the "Moog" width)
/ C4 = 261.6Hz — a representative lead note
N: 22050
T: !N

/ Two detuned VCOs (standard Moog modular patch)
F: 261.6*(6.28318%44100)
G: 264.0*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)

/ Sawtooth spectrum — the Moog VCO sawtooth
A: 1 .5 .333 .25 .2 .167 .143
S: (P $ A)
U: (Q $ A)
V: (S+U)*.5

/ The Moog ladder LP filter with envelope control
/ Filter envelope: fast attack (cutoff opens rapidly), then decays slowly
/ Start at ~500Hz, peak at ~2500Hz, decay back to ~600Hz
/ We model this with a blend of bright and dark filtered signals
O: e(T*(0-20%N))
B: 0.35 f V
I: 0.6 f V
/ Envelope opens the filter: bright at attack, dark at sustain
D: (O*I)+((1-O)*B)

/ VCA envelope: full sustain for the lead (hold the note)
E: e(T*(0-6.9%N))

/ Moog "warmth" — the slight saturation from driving the ladder filter
X: d(D*1.3)

W: w E*X
