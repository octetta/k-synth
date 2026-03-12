/ NAP body slam — low impact, distorted boom, no pitch
N: 11025
T: !N
/ very low filtered noise
R: r T
E: e(T*(0-8%N))
L: 0.03 f R
B: 0.03 f L
/ add sub tone
F: 60*(6.28318%44100)
P: +\(N#F)
S: e(T*(0-6%N))*s(P)
W: w d((E*B*1.5+S*0.6)*3.0)
