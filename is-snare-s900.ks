/ InSoc Snare — Akai S-900 user sample quality
/ Paul Robb loaded custom samples into the S-900 alongside SP-12 presets
/ The S-900 snare would have been a "better" sample — 44.1kHz, cleaner
/ Used for main snare on some tracks where they wanted more presence
/ Character: clean acoustic snare sample (no 12-bit grit)
/ 220ms
N: 9702
T: !N

B: e(T*(0-30%N))
F: 175*(6.28318%44100)
G: 215*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
O: s P
I: s Q
S: B*((O+I)*.5)

E: e(T*(0-6.9%N))
R: r T
L: 0.35 f R
U: E*L

V: e(T*(0-300%N))
K: r T
/ S-900: no static Chebyshev — full range available
A: V*(K-(0.5 f K))

/ No 12-bit quantize — S-900 is clean
W: w (S*.6)+(U*.55)+(A*.35)
