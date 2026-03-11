/ Moroder Bass — E=MC2 (1979), Roland System 700 + MC-8
/ The System 700 had different character from Moog: Roland's IR3109 filter
/ Less "fat" than Moog ladder — tighter, more precise, slightly thinner
/ "The sound is strictly moogs, memory banks, and moroder" (actually System 700)
/ The System 700 sequenced at 16th notes via MC-8 — fully programmed
/ More precision, less character than the IFL Moog bass
/ C2 = 65.4Hz
N: 7056
T: !N

F: 65.4*(6.28318%44100)
G: 65.8*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)

/ Roland System 700 VCO: sawtooth (similar to Moog but slightly different wave)
A: 1 .5 .333 .25 .2 .167
S: P $ A
U: Q $ A
V: (S+U)*.5

/ Roland IR3109 filter: similar to Moog ladder but slightly different character
/ A bit tighter, less euphonic saturation = use single LP pass (not double)
/ ct=0.1 → ~702Hz — the Roland filter was often set in this range
C: 0.1 f V

/ Tighter VCA envelope — the System 700 precision
E: e(T*(0-15%N))

W: w E*C
