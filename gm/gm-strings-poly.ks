/ Moroder Strings — Polymoog / Prophet-5 string pad (later productions)
/ E=MC2 (1979): Roland System 700, Polymoog, Prophet-5 used
/ The Prophet-5 string pad: two sawtooth VCOs, LP filter open, long release
/ More "lush" than the Solina — the poly synth ensemble string sound
/ Character: fatter, warmer, slightly slower attack than Solina
/ C4 = 261.6Hz
N: 44100
T: !N

/ Two detuned sawtooths — the Prophet-5 unison approach
F: 261.6*(6.28318%44100)
G: 263.2*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)

/ Prophet-5 sawtooth (CEM oscillator): full harmonic content
A: 1 0.5 0.333 0.25 0.2 0.167 0.143
S: P $ A
U: Q $ A
V: (S+U)*.5

/ CEM filter (Prophet-5): similar to Moog ladder — 4-pole LP
/ For strings: cutoff very open (ct=0.7), low resonance
/ Apply twice to approximate 4-pole character
C: 0.7 f V
X: 0.7 f C

/ Long release envelope — the string sustain
E: e(T*(0-3%N))

W: w E*X
