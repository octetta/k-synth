/ 808 Cowbell — 735Hz + 850Hz cluster, 900ms
/ N=39690
N: 39690
T: !N
E: e(T*(0-6.9%N))
/ two close tones at measured frequencies
/ fast initial decay + slow ring
F: 735*(6.28318%44100)
G: 850*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
/ harmonic content from $ — slightly square-ish
A: 1 0 0.3 0 0.15
J: P $ A
K: Q $ A
M: J+K*.8
W: w E*M
