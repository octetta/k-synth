/ 808 Snare — 170Hz body dominant, noise tail, 280ms
/ N=12348
N: 12348
T: !N
/ body: 170Hz + 183Hz detuned pair, decay ~20ms
B: e(T*(0-25%N))
F: 170*(6.28318%44100)
G: 183*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
S: B*(s P+s Q)*.5
/ noise tail: full duration — lower in mix than body at start
E: e(T*(0-6.9%N))
R: r T
U: E*R
/ snap
V: e(T*(0-80%N))
K: r T
/ body is primary (*.7), noise secondary (*.4)
W: w (S*.7)+(U*.4)+(V*K*.2)
