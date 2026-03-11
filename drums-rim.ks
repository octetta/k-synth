/ 808 Rimshot — 1800Hz dominant, dead in 20ms
/ N=2646 (60ms)
N: 2646
T: !N
/ very fast overall decay
E: e(T*(0-6.9%N))
/ 1800Hz tone — the dominant frequency
F: 1800*(6.28318%44100)
P: +\(N#F)
S: (s P)
/ broadband noise crack
R: r T
L: 0.15 f R
H: R-L
W: w E*(S*.6+H*.5)
