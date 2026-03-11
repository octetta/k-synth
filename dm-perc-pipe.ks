/ DM Metal Pipe Hit — THE signature Construction Time Again sound
/ Wilder recording band members hitting metal pipes at a construction site
/ Character: bright inharmonic attack, long clean decay, clearly pitched
/ This is a musical pitched-ish percussion instrument, not a drum
/ Tuned around E4 / ~330Hz with inharmonic partials
/ 800ms — these ring for a long time
N: 35280
T: !N
E: e(T*(0-6.9%N))

/ Real metal tube/pipe: first few modes are roughly harmonic then diverge
/ Mode 1: 330Hz (fundamental)
/ Mode 2: 660Hz (2x — nearly harmonic)
/ Mode 3: 1045Hz (3.17x — starts to go inharmonic)
/ Mode 4: 1540Hz (4.67x — more inharmonic)
/ Mode 5: 2130Hz (6.45x)
F: 330*(6.28318%44100)
G: 660*(6.28318%44100)
J: 1045*(6.28318%44100)
K: 1540*(6.28318%44100)
L: 2130*(6.28318%44100)

P: +\(N#F)
Q: +\(N#G)
R: +\(N#J)
S: +\(N#K)
U: +\(N#L)

/ Each mode decays at a different rate — higher modes decay faster
/ This creates the characteristic "metallic" tone color evolution over time
A: (s P)*e(T*(0-6.9%N))
B: (s Q)*e(T*(0-9%N))
C: (s R)*e(T*(0-12%N))
D: (s S)*e(T*(0-16%N))
H: (s U)*e(T*(0-22%N))

/ Strike transient: broadband short noise
V: e(T*(0-400%N))
X: r T
Z: V*X

W: w (A*.9)+(B*.5)+(C*.35)+(D*.2)+(H*.1)+(Z*.3)
