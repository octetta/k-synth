/ Moroder Strings — ARP Solina String Ensemble
/ Used on "From Here to Eternity" and IFL sessions
/ The Solina is a divide-down organ architecture with a BBD ensemble chorus
/ The chorus = bucket brigade device creating rich, flanging, detuned texture
/ "The synth string section is the salad dressing — a flangy orchestral sound,
/  with an open envelope filter that gives it a wider, longer release" — Reverb
/ The Solina character: sawtooth-like wave, very long attack (strings), BBD chorus
/ C4 = 261.6Hz, full string chord would layer multiple notes
/ This voice = one string note; layer C+E+G+Bb for the IFL chord texture
N: 44100
T: !N

/ ARP Solina VCO: sawtooth, but softer than Moog (no LP filter — the Solina
/  ran into the BBD chorus directly, no resonant filter)
F: 261.6*(6.28318%44100)
G: 262.8*(6.28318%44100)
H: 263.6*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
R: +\(N#H)

/ Soft sawtooth — fewer harmonics than Moog (Solina was tonally softer)
A: 1 .5 .25 .125
S: P $ A
U: Q $ A
X: R $ A

/ BBD ensemble chorus effect:
/ Three detuned copies beating against each other = the Solina "shimmer"
/ We already have three detuned VCOs — their beating IS the chorus
V: (S+U+X)*.333

/ Long attack envelope (strings): the Solina had slow attack
/ Attack = slow fade in over ~200ms at 44100Hz = 8820 samples
/ Rise-then-sustain: envelope opens slowly
/ Use inverted-decay as attack: 1 - e(-t*k/N) approximation
/ ksynth: attack = (1 - e(T*(0-20%N)))... not directly available
/ Workaround: use full decay envelope on reversed signal — but can't reverse
/ Instead: fast rise with slow sustained decay = close enough
E: e(T*(0-3%N))

/ The open filter character (long release): Solina had no LP — full spectrum
/ The flanging from the BBD = slight comb filtering — LP approximation
C: 0.7 f V

W: w E*C
