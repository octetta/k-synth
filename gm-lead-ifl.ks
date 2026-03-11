/ Moroder Inner Lead — "I Feel Love" high melodic line, Moog modular
/ The inner melodic phrases that appear between vocals in IFL
/ Higher pitch, faster envelope, more nasal character
/ A4 = 440Hz, 300ms
N: 13230
T: !N

F: 440*(6.28318%44100)
G: 443*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)

A: 1 .5 .333 .25 .2 .167 .143
S: (P $ A)
U: (Q $ A)
V: (S+U)*.5

/ IFL melodic lead: moderate filter, fast attack then sustain
/ The inner lead was less dramatic than Chase — more melodic, warm
O: e(T*(0-15%N))
B: 0.1 f V
I: 0.4 f V
D: (O*I)+((1-O)*B)

E: e(T*(0-6.9%N))
X: d(D*1.2)

W: w E*X
