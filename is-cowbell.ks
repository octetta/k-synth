/ InSoc Cowbell — E-MU SP-12 "Cowbell" preset
/ SP-12 had a cowbell preset in ROM — static Chebyshev filter
/ Used as an accent hit in dance patterns
/ The SP-12 cowbell sample had a more "metallic box" quality
/ 400ms
N: 17640
T: !N
E: e(T*(0-6.9%N))

/ SP-12 cowbell: similar frequencies to 808 but at 12-bit
F: 562*(6.28318%44100)
G: 845*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
A: 1 0 0.3 0 0.15
J: P $ A
K: Q $ A
M: J+K*.8

/ Static Chebyshev filter — cowbell channel
C: 0.6 f M
/ 12-bit lo-fi
Q: 4096 v (E*C)
W: w Q
