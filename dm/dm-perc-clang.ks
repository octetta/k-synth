/ DM Metal Clang — Construction Time Again industrial bang
/ "Everything Counts", "People Are People" metal percussion
/ Character: massive transient, higher inharmonic content, faster decay
/ More aggressive than pipe — this is something being hit hard
/ 500ms
N: 22050
T: !N
E: e(T*(0-6.9%N))

/ Higher pitched metal object — maybe 550Hz base
/ Inharmonic ratios of a struck plate (not tube)
/ Plate modes: ~1.0, 1.57, 2.16, 2.92 (Chladni patterns)
F: 550*(6.28318%44100)
G: 863*(6.28318%44100)
J: 1188*(6.28318%44100)
K: 1606*(6.28318%44100)

P: +\(N#F)
Q: +\(N#G)
R: +\(N#J)
S: +\(N#K)

/ Even faster decay per mode
A: (s P)*e(T*(0-8%N))
B: (s Q)*e(T*(0-14%N))
C: (s R)*e(T*(0-20%N))
D: (s S)*e(T*(0-28%N))

/ Harder strike = more broadband noise in transient
V: e(T*(0-200%N))
X: r T
U: V*X

W: w (A*.9)+(B*.6)+(C*.4)+(D*.2)+(U*.5)
