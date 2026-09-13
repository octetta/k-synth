/ NAP zap perc — electric shock burst, very short
N: 3307
T: !N
/ fast pitch dive from high to nothing
F: 10+2000*e(T*(0-200%N))
D: F*((p 2)%(p 0))
P: +\D
S: s P
E: e(T*(0-30%N))
/ noise layer
R: r T
G: e(T*(0-60%N))
H: R-(0.2 f R)
W: w d((E*S*0.6+G*H*0.5)*3.0)
