/ NIN Kick 2 — shorter, tighter, more "programmed" feel
/ The March of the Pigs / Wish type kick — not as long-tailed as Closer
/ 200ms
N: 8820
T: !N
E: e(T*(0-6.9%N))
F: 50+100*e(T*(0-60%N))
D: F*(6.28318%44100)
P: +\D
O: s P
/ Hard clip immediately
C: d(O*4)
/ Very fast noise transient — the programmed click
V: e(T*(0-500%N))
R: r T
A: V*R*.4
/ Sub underneath
G: 42*(6.28318%44100)
Q: +\(N#G)
B: (s Q)*e(T*(0-4%N))
W: w (C*E*.9)+(A*.2)+(B*.3)
