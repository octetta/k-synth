/ Kraftwerk Cymbal — Simmons SDS-V cymbal module (digital samples + analog filter)
/ Grokipedia: "Optional cymbal modules employed digital sampling enhanced
/ by analog filters for metallic effects"
/ Character: inharmonic metallic content, LP-filtered for warmth, long
/ The analog filter on the digital cymbal sample = less harsh than raw digital
/ 1200ms
N: 52920
T: !N
E: e(T*(0-6.9%N))

/ Inharmonic oscillators — same physics but triangle-based
/ Triangle adds warmth vs pure sine
B: 2900*(6.28318%44100)
P: +\(N#(B*1.000))
Q: +\(N#(B*1.342))
R: +\(N#(B*1.200))
S: +\(N#(B*1.618))
U: +\(N#(B*1.478))

/ Triangle spectrum for each partial
A: 1 0 -.111 0 .04
J: P $ A
K: Q $ A
L: R $ A
M: S $ A
X: U $ A
Z: J+K+L+M+X

/ SDS-V cymbal module: LP filter shapes the digital source for warmth
/ The SSM2044 LP at ~6kHz softens the harsh digital edges
Y: 0.8 f Z

/ 1-bit shimmer — the metallic "air" above the LP cutoff
C: m T
G: e(T*(0-4%N))

W: w (E*Y*.8)+(G*C*.25)
