/ InSoc Crash — E-MU SP-12 "Crash" preset
/ SP-12 ride/crash channels: UNFILTERED (no SSM2044, no Chebyshev)
/ Like the hi-hat channels: raw 12-bit sample, no reconstruction filter
/ The imaging artefacts above 13kHz are the character
/ 1000ms — SP-12 crashes sustained well
N: 44100
T: !N
E: e(T*(0-6.9%N))

/ Inharmonic oscillator stack — same physics
B: 3000*(6.28318%44100)
P: +\(N#(B*1.000))
Q: +\(N#(B*1.342))
R: +\(N#(B*1.200))
S: +\(N#(B*1.618))
U: +\(N#(B*1.478))
A: 1 0 0.5 0 0.25
J: P $ A
K: Q $ A
L: R $ A
M: S $ A
X: U $ A
Z: J+K+L+M+X

/ Unfiltered — no LP, all content passes
/ 1-bit noise: the imaging artefact character
C: m T
G: e(T*(0-4%N))

/ 12-bit quantize: the SP-12 grit on an unfiltered channel = very apparent
Y: 4096 v Z
W: w (E*Y*.75)+(G*C*.35)
