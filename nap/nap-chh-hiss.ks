/ NAP hi-hat hiss — dirty open hat, long metallic decay
N: 13230
T: !N
R: r T
E: e(T*(0-4%N))
/ modulated filter to add shimmer
F: 0.6+0.2*s(+\(N#(7*(6.28318%44100))))
H: R-(0.2 f R)
B: F f H
W: w d(E*B*1.8)
