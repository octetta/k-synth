/ Moroder Arpeggiated Synth — Moog modular sequenced, "I Feel Love" inner parts
/ "The Chase" "The hypnotic repetitive arpeggiated figures"
/ The inner arpeggiated parts in IFL: Moog modular VCO through LP, sequenced
/ Fast staccato notes — shorter than the bass pluck, higher pitch
/ Moroder confirmed: "some of the chords—which were difficult to do because
/  I had to do each note of a chord separate on different tracks"
/ So this represents ONE note of a chord note in the arpeggio
/ C5 = 523.2Hz, 120ms gate
N: 5292
T: !N

F: 523.2*(6.28318%44100)
G: 526.2*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)

/ Moog sawtooth
A: 1 0.5 0.333 0.25 0.2 0.167 0.143
S: P $ A
U: Q $ A
V: (S+U)*.5

/ Moog LP: tighter than strings — this is the percussive inner arp part
/ Fast filter envelope opening and closing with the note
/ At ~2kHz (ct=0.28) with fast decay to ~400Hz (ct=0.057)
O: e(T*(0-30%N))
B: 0.057 f V
I: 0.28 f V
D: (O*I)+((1-O)*B)

E: e(T*(0-20%N))

W: w E*D
