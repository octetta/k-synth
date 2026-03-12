/ NAP snare rimshot — sharp metallic crack, no body
N: 5292
T: !N
/ two high clashing tones
A: 400*(6.28318%44100)
B: 555*(6.28318%44100)
P: +\(N#A)
Q: +\(N#B)
C: e(T*(0-60%N))
S: C*(s(P)+s(Q)*0.7)
/ HP noise snap
R: r T
E: e(T*(0-80%N))
H: R-(0.15 f R)
W: w d((S*0.5+E*H*0.8)*2.5)
