/ NIN Crash — long, washy, distorted
/ More noise-dominant than cymbal-tonal
/ 1500ms with noise floor that lingers
N: 66150
T: !N
E: e(T*(0-6.9%N))

/ Inharmonic oscillators — same ratios as 808 but distorted harder
B: 2800*(6.28318%44100)
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

/ Clip the tonal stack — distorted cymbal character
O: d(Z*.4)

/ 1-bit noise shimmer — the harsh industrial shimmer
C: m T
/ second decay for noise tail — slightly different rate
G: e(T*(0-5%N))

/ Low noise floor — the room rumble/wash
I: r T
F: 0.04 f I
V: e(T*(0-3%N))

W: w (E*O*.6)+(G*C*.5)+(V*F*.15)
