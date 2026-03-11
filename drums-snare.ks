/ 909-style Snare
/ 400ms at 44100 = 17640 samples

N: 17640
T: !N
/ body: two detuned sines, fast decay ~10ms
B: e(T*(0-40%N))
F: 200*(6.28318%44100)
G: 180*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
S: B*(s P+s Q)*.5
/ noise tail: full duration
E: e(T*(0-6.9%N))
R: r T
U: E*R
/ snap transient: very fast noise burst
V: e(T*(0-80%N))
K: r T
/ explicit parens around each term for correct linear mix
W: w (S*.25)+(U*.8)+(V*K*.3)
