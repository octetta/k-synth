/ Moroder Open Hi-Hat — Moog modular, "I Feel Love" (1977)
/ Same white noise → envelope as CHH, but longer decay
/ The Moog VCA with longer release: the open hat sustain
/ 250ms
N: 11025
T: !N

R: r T
E: e(T*(0-6.9%N))

/ Same bright LP as CHH
C: 0.7 f R

/ Slightly longer ring for the open sustain
G: e(T*(0-2%N))

W: w (E*C*.85)+(G*R*.15)
