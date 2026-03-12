/ NAP clap — hard slap, almost like a gunshot, no warmth
N: 6615
T: !N
/ three staggered noise bursts, sharp attack
R: r T
A: e(T*(0-50%N))
X: r T
B: e(T*(0-30%N))
Y: r T
C: e(T*(0-15%N))
/ HP each burst
H: R-(0.08 f R)
I: X-(0.08 f X)
J: Y-(0.08 f Y)
/ stack with timing offsets baked into decay
W: w d((A*H*0.5+B*I*0.8+C*J)*2.5)
