/ 808 Crash — measured from real sample
/ The sample is broadband noise, not tonal — no oscillators
/ Spectral centroid descends 7kHz->3kHz over ~400ms as high content decays
/ Effective -60dB at ~1025ms, total 1500ms
N: 66150
T: !N

/ noise source — 1-bit metallic gives the dense inharmonic character
/ white noise alone is too smooth; m gives the right grainy texture
M: m T
R: r T
/ blend: mostly metallic, some white for density
X: (M*.7)+(R*.3)

/ attack layer: highpass white noise, fast decay ~80ms
/ models the 6-9kHz transient measured in the sample
A: e(T*(0-60%N))
H: X - (0.7 f X)
/ 0.7 f = ~5kHz lowpass, so X - that = highpass above ~5kHz

/ body layer: the same noise through a gentler highpass
/ slower decay matches the measured ~1025ms -60dB point
E: e(T*(0-6.9%N))
/ lowpass around 4kHz — keeps body from being too bright
L: 0.6 f X
/ subtract a lower lowpass to get a band centered around 2-4kHz
B: L - (0.1 f X)

/ tail layer: low-mid content, slowest decay
/ models the 1.2-2.5kHz residue measured at 600ms+
G: e(T*(0-4.5%N))
/ narrow lowpass around 2kHz
K: 0.15 f X

W: w (A*H*.5)+(E*B*.8)+(G*K*.4)
