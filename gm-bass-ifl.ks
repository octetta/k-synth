/ Moroder Bass — "I Feel Love" (1977), Moog IIIP modular
/ "I told him: I need two Cs and a G and a B-flat"
/ "The original bassline was one [dun dun dun dun], then when we started to mix
/  we delayed it and gave it a totally new sound"
/ The "dum-dum-dum-dum": sawtooth VCO through Moog ladder LP
/ Sequenced: C note (65.4Hz), moving up through G, Bb
/ This voice represents C (root): the listener plays different notes via transposition
/ TWO VCOs slightly detuned (Moog modular had multiple VCO modules)
/ Moog ladder LP at ~600Hz with short AR envelope — the cutoff is KEY
/ Short decay envelope on VCA: each note is plucked, staccato
/ "The sound of the future" — clean, fat, no warm wobble
/ 200ms — the note gate is shorter than the full voice (staccato character)
N: 8820
T: !N

/ C2 = 65.4Hz — the fundamental sequencer note
/ Two VCOs, slightly detuned (3 cents) = the Moog modular multi-VCO richness
F: 65.4*(6.28318%44100)
G: 65.7*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)

/ Sawtooth wave from ksynth perspective: use spectrum approximation
/ Sawtooth = 1/n harmonics: 1, 0, .5, 0, .333, 0, .25...
/ We use a simpler approach: two-oscillator mix (Moog used sawtooth VCOs)
A: 1 .5 .333 .25 .2 .167 .143
S: P $ A
U: Q $ A

/ Two VCO mix = thicker, slight beating
V: (S+U)*.5

/ Moog ladder LP: 4-pole 24dB/oct, cutoff ~600Hz (ct=0.086)
/ The reverb forum confirms ~600Hz with ~33% resonance
/ ksynth f verb is ~2-pole; we cascade two LP passes to approximate 4-pole
C: 0.086 f V
X: 0.086 f C

/ VCA envelope: short gate, staccato — the sequenced pluck character
/ Very fast attack (instant), decay ~120ms
E: e(T*(0-12%N))

/ The AMS delay that created the 16th-note version:
/ We can't do delay in ksynth — the delay is in the sequencer/playback layer
/ This voice is the dry note; the user sequences the delay externally

W: w E*X
