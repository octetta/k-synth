/ Kraftwerk Closed Hi-Hat — Maestro Rhythm-King / Farfisa Rhythm Unit style
/ "The Model", "Numbers" — very tight, metallic, precise
/ Rhythm-King hi-hat = bandpassed white noise, very short decay
/ NOT the 1-bit metallic of NIN — cleaner, more clinical
/ 80ms
N: 3528
T: !N

/ Very fast double-decay: initial crack then quick tail
E: e(T*(0-6.9%N))
C: e(T*(0-25%N))

/ Bandpassed noise: Rhythm-King used simple RC filter on noise
/ HP above 3kHz, some content up to 8kHz
R: r T
L: 0.4 f R
H: R-L

/ The Synthanorma "clipped high resonant frequencies" — slight resonance
/ adds a metallic edge to the otherwise clean noise
M: 0.5 0.3 f R
Z: M-(0.4 f M)

W: w C*E*(H*.6+Z*.4)
