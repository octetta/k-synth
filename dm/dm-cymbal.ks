/ DM Cymbal — clean inharmonic shimmer, long sustain
/ Not distorted — the EMU/Synclavier samples were clean
/ Very different from NIN crash: musical, controlled, clean ring
/ 1200ms
N: 52920
T: !N
E: e(T*(0-6.9%N))

/ Standard inharmonic cymbal ratios — same physics, no distortion applied
B: 3200*(6.28318%44100)
P: +\(N#(B*1.000))
Q: +\(N#(B*1.342))
R: +\(N#(B*1.200))
S: +\(N#(B*1.618))
U: +\(N#(B*1.478))

/ Amplitude spectrum — odd harmonics, clean
A: 1 0 0.5 0 0.25
J: P $ A
K: Q $ A
L: R $ A
M: S $ A
X: U $ A
Z: J+K+L+M+X

/ 1-bit shimmer — metallic but NOT distorted through saturation
/ Just the natural 1-bit character
C: m T
/ Slower decay for shimmer than for tonal stack
G: e(T*(0-5%N))

W: w (E*Z*.8)+(G*C*.3)
