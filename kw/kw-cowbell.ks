/ Kraftwerk Cowbell — Maestro Rhythm-King cow bell
/ "Trans-Europe Express" — the Rhythm-King had a cowbell voice
/ Character: two close triangle-ish tones, medium decay, clean
/ More naive/mechanical than 808 cowbell — the Rhythm-King was simpler
/ 600ms
N: 26460
T: !N
E: e(T*(0-6.9%N))

/ Rhythm-King cowbell: two triangle tones
/ Measured 808: 562+845Hz, but Rhythm-King tuned differently
/ Slightly lower, rounder character
F: 520*(6.28318%44100)
G: 800*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
A: 1 0 -.111 0 0.04
O: P $ A
I: Q $ A

/ Slight clip — Synthanorma treatment
S: d((O+I*.7)*1.3)

W: w E*S
