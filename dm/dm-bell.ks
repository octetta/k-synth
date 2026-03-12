/ DM Bell/Synclavier tone — the melodic percussion element
/ The bell-like timbres from "Synclavier additive synthesis" (Wilder)
/ Tuned to ~A4 440Hz — use in patterns as pitched accent
/ Long resonant decay: 1200ms
N: 52920
T: !N
E: e(T*(0-6.9%N))

/ Synclavier bell synthesis: fundamental + inharmonic partials
/ Synclavier specialty was precise additive synthesis of bell tones
/ Ratios approximate real bell: 1.0, 2.756, 5.404, 8.933, 13.345
F: 440*(6.28318%44100)
G: 1213*(6.28318%44100)
J: 2378*(6.28318%44100)
K: 3930*(6.28318%44100)

P: +\(N#F)
Q: +\(N#G)
R: +\(N#J)
S: +\(N#K)

/ Partials decay at different rates — higher modes faster
A: (s P)*e(T*(0-6.9%N))
B: (s Q)*e(T*(0-10%N))
C: (s R)*e(T*(0-15%N))
D: (s S)*e(T*(0-22%N))

W: w (A*.9)+(B*.6)+(C*.3)+(D*.15)
