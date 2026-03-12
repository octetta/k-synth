/ InSoc Clap — E-MU SP-12 "Clap" preset
/ SP-12 clap channel: static Chebyshev filter
/ "What's On Your Mind" chorus clap — very prominent, slightly gritty
/ SP-12 clap had a distinctive "snappy" quality from the 12-bit sample
/ 150ms
N: 6615
T: !N

/ Three noise bursts — acoustic clap sample behaviour
R: r T
A: 0.35 f R
B: 0.06 f R
C: A-B
X: w (T*e(T*(0-30%N)))

J: r T
D: 0.35 f J
F: 0.06 f J
G: D-F
Y: w (T*e(T*(0-12%N)))

K: r T
/ SP-12 static Chebyshev on clap channel
L: 0.35 f K
U: K-L
Z: w (T*e(T*(0-6%N)))

P: (X*C*.4)+(Y*G*.7)+Z*U

/ 12-bit quantize the whole clap — the SP-12 lo-fi texture
W: w 4096 v P
