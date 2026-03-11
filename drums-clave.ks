/ Clave
/ 80ms at 44100 = 3528 samples

N: 3528
T: !N
E: e(T*(0-6.9%N))
F: 2400*(6.28318%44100)
G: 2650*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
S: s P+s Q*.7
V: e(T*(0-16%N))
R: r T
W: w E*(S+V*R*.2)
