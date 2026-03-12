/ NIN Body Hit — the industrial THUD
/ A low distorted thump — used for accents, the "industrial body slam"
/ Think: Ruiner, March of the Pigs accent hits
/ Short, massive sub + noise, heavily clipped
N: 8820
T: !N
E: e(T*(0-6.9%N))

/ Very low fixed tone — not pitch-swept, just a wall of low end
F: 55*(6.28318%44100)
P: +\(N#F)
O: s P

/ Clip hard — tape saturation on sub = the NIN "thud"
S: d(O*5)

/ Sub reinforcement
B: 40*(6.28318%44100)
Q: +\(N#B)
I: (s Q)*e(T*(0-5%N))

/ Noise component — room boom
R: r T
C: 0.05 f R
V: e(T*(0-20%N))

W: w (E*S*.9)+(I*.3)+(V*C*.2)
