/ Moroder Bass — "Chase" / Midnight Express (1978), Minimoog
/ "The bass parts were done with my MiniMoog. Anything that sounds sequenced
/  wasn't. For the bass parts I would play 8th notes and an AMS digital delay
/  would play the 16th notes." — Greg Mathieson (session keyboardist)
/ Minimoog bass: single VCO sawtooth, Moog ladder LP
/ Slightly different character from IFL: one VCO, more aggressive cutoff
/ The chase bassline is more intense — higher resonance feeling
/ C2 note, 180ms gate
N: 7938
T: !N

F: 65.4*(6.28318%44100)
P: +\(N#F)

/ Sawtooth via spectrum
A: 1 0.5 0.333 0.25 0.2 0.167 0.143
S: P $ A

/ Minimoog LP: tighter cutoff (more resonant character = more filter body)
/ At ~500Hz (ct=0.07) — slightly darker than IFL
C: 0.07 f S
X: 0.07 f C

/ Moog VCA envelope: fast attack, medium decay
/ The "Chase" bass was slightly more punchy, less staccato
E: e(T*(0-9%N))

W: w E*X
