/ InSoc "Slam" hit — the big layered accent hit
/ "What's On Your Mind", "Land of the Blind" — the big downbeat hit
/ InSoc technique: layer SP-12 kick + SP-12 tom (tuned down) + S-900 sample
/ This single voice represents that layered combo
/ 300ms
N: 13230
T: !N
E: e(T*(0-6.9%N))

/ Layer 1: SP-12 kick body
F: 55+80*e(T*(0-70%N))
D: F*(6.28318%44100)
P: +\D
S: 4096 v (s P)

/ Layer 2: "way tuned-down SP-12 tom" at ~65Hz, dynamic VCF
G: 65*(6.28318%44100)
M: +\(N#G)
O: e(T*(0-200%N))
I: 0.3 f (s M)
J: 0.04 f (s M)
B: (O*I)+((1-O)*J)
X: 4096 v B

/ Layer 3: transient/click from SP-12 beater
V: e(T*(0-500%N))
R: r T
C: 0.15 f R
K: V*C

/ Sub from the Moog Prodigy layering (clean sine, very low)
A: 38*(6.28318%44100)
Q: +\(N#A)
Z: (s Q)*e(T*(0-4%N))

W: w (E*S*.7)+(E*X*.5)+(K*.3)+(Z*.3)
