/ Moroder Kick — "From Here to Eternity", "I Feel Love" (1977)
/ Keith Forsey played just the bass drum in an isolation booth
/ "The Moog couldn't give me a punch sound, it would give me oomph instead of dum"
/ Modeled as: acoustic kick body, very deep sub, short punchy decay
/ Moroder wanted it to anchor the sequenced Moog bass — so it needed sub weight
/ The "oomph" the Moog could do is what became the tom/sub layer here
/ 300ms
N: 13230
T: !N
E: e(T*(0-6.9%N))

/ Acoustic kick body: fast pitch sweep, punchy
/ Forsey's real kick had the thwack the Moog lacked
F: 50+100*e(T*(0-100%N))
D: F*(6.28318%44100)
P: +\D
S: s P

/ The "oomph" the Moog COULD do — the sub layer
/ This is what Moroder described: round, swelling low-frequency weight
/ Moog modular VCO: sine-like (heavily LP-filtered sawtooth through Moog ladder)
/ Two detuned VCOs → Moog LP at ~80Hz = pure sub sine
G: 55*(6.28318%44100)
M: +\(N#G)
O: s M
/ Moog ladder LP on sub: very tight (ct=0.02 → ~140Hz)
U: 0.02 f O
B: U*e(T*(0-3%N))

/ Beater transient: the acoustic click
V: e(T*(0-600%N))
R: r T
/ HP to isolate the click
C: R-(0.1 f R)
K: V*C

W: w (E*S*.8)+(B*.4)+(K*.25)
