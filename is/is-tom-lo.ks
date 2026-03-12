/ InSoc Lo Tom — E-MU SP-12 tom preset
/ Tuned low: ~100Hz — used as the "way tuned down" tom in the kick layer
/ Can also be used standalone for low tom hits
/ 500ms
N: 22050
T: !N
E: e(T*(0-6.9%N))

F: 100*(6.28318%44100)
P: +\(N#F)
S: s P

O: e(T*(0-200%N))
I: 0.3 f S
J: 0.04 f S
D: (O*I)+((1-O)*J)
Q: 4096 v D

V: e(T*(0-300%N))
R: r T
C: 0.1 f R
K: V*C

W: w (E*Q*.85)+(K*.2)
