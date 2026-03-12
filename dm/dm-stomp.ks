/ DM Stomp — Personal Jesus "flight case" body hit
/ Flood: "maybe we can make the footstomp a bit bigger"
/ then added room track underneath
/ Used as a heavy downbeat accent — not a kick, a stomp
/ Very low, very short, very physical
/ 300ms
N: 13230
T: !N

/ The body impact — very low broadband thud
/ A flight case is a large wooden/metal box: resonates ~60-80Hz
E: e(T*(0-6.9%N))
F: 65*(6.28318%44100)
P: +\(N#F)
O: s P

/ Sub reinforcement — the floor coupling
G: 38*(6.28318%44100)
Q: +\(N#G)
I: (s Q)*e(T*(0-5%N))

/ "Room mic" layer: the big thumping noise
R: r T
L: 0.05 f R
C: L*e(T*(0-15%N))

/ Initial broadband transient (the impact)
V: e(T*(0-300%N))
K: r T
A: V*K*.5

W: w (E*O*.85)+(I*.3)+(C*.35)+(A*.2)
