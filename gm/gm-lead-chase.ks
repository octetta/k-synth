/ Moroder Lead — "Chase" / Midnight Express (1978), Moog modular
/ The Chase lead: that relentless, rising, filter-swept Moog lead line
/ Character: sawtooth with WIDE filter sweep opening as the note plays
/ The sweep is faster and more dramatic than the sustained IFL lead
/ The "Giorgio by Moroder" Daft Punk analysis: 168Hz→1000Hz filter sweep
/ at increasing resonance — that's the Chase lead character
/ C4 = 261.6Hz, 500ms with dramatic filter opening
N: 22050
T: !N

F: 261.6*(6.28318%44100)
G: 264.0*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)

/ Moog sawtooth
A: 1 0.5 0.333 0.25 0.2 0.167 0.143
S: (P $ A)
U: (Q $ A)
V: (S+U)*.5

/ The dramatic Chase filter sweep:
/ Starts tight (~168Hz = ct=0.024), sweeps to open (~1000Hz = ct=0.143)
/ Fast sweep: envelope decays quickly (the filter OPENS as envelope CLOSES here)
/ This inverts the usual pattern: use (1-decay) = opening sweep
O: e(T*(0-8%N))
B: 0.024 f V
I: 0.143 f V
/ O starts at 1, falls to 0: when O=1 → dark filter; when O=0 → open filter
/ So D starts dark and opens — matches the Chase rising sweep character
D: (O*B)+((1-O)*I)

/ The "increasing resonance" from the Daft Punk analysis:
/ Approximated by driving the filter input harder (saturation = Moog warmth)
X: d(D*1.5)

E: e(T*(0-6.9%N))
W: w E*X
