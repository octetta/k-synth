/ Moroder Vocoder carrier — Moog vocoder / EMS vocoder 3000
/ Moroder used the Moog vocoder (and possibly EMS VSM-201 or EMS vocoder 3000)
/ The vocoder carrier: a sawtooth buzz at speaking pitch
/ This represents the carrier signal going INTO the vocoder
/ The carrier determines the timbral base of the vocoder output
/ Characteristic: raw buzzy sawtooth, no filtering (vocoder does the filtering)
/ "He seems to run vocals through the modulars" — Gearspace forum
/ F0 carrier ≈ 120Hz (male speaking fundamental)
N: 22050
T: !N

/ Sawtooth carrier at vocal fundamental
F: 120*(6.28318%44100)
P: +\(N#F)

/ Unfiltered sawtooth: rich harmonic content feeds all vocoder bands
A: 1 .5 .333 .25 .2 .167 .143 .125 .111 .1
S: P $ A

/ VCA envelope: sustained for the duration of the vocoded phrase
E: e(T*(0-6.9%N))

/ No filter — the vocoder provides the spectral shaping
W: w E*S
