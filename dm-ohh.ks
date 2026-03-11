/ DM Open Hi-Hat — longer metal shimmer, clean ring
/ Like a thin metal plate ringing — not a conventional cymbal
/ 450ms
N: 19845
T: !N
E: e(T*(0-6.9%N))

M: m T
R: r T
L: 0.6 f R
H: R-L

/ Secondary slower decay — the "ring" tail of the metal plate
F: e(T*(0-3%N))

/ Tonal component: a high inharmonic partial for the shimmer character
/ ~6kHz partial that decays slower than the noise
G: 6200*(6.28318%44100)
P: +\(N#G)
O: (s P)*e(T*(0-8%N))

W: w (E*(M*.4+H*.4))+(F*O*.2)
