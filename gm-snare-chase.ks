/ Moroder Snare — "Chase" (1978), "From Here to Eternity"
/ "He used the Moog Modular for everything but the snare drum"
/ The Chase snare was an actual drum sample/loop from Keith Forsey sessions
/ More acoustic snap, less synthesized than the IFL snare
/ Brighter, more acoustic character — the real drummer snare
/ 200ms
N: 8820
T: !N

/ Pitched body: acoustic snare crack ~200Hz + 250Hz
B: e(T*(0-30%N))
F: 200*(6.28318%44100)
G: 250*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
O: s P
I: s Q
S: B*((O*.6+I*.4))

/ White noise: Moog white noise, Moog LP shaped envelope
E: e(T*(0-6.9%N))
R: r T
/ Brighter LP than IFL snare (more crack, less body)
L: 0.5 f R
U: E*L

/ Snap: HP component, fast transient
V: e(T*(0-300%N))
K: r T
A: V*(K-(0.1 f K))

W: w (S*.4)+(U*.7)+(A*.4)
