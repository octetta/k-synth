/ InSoc Mid Tom — E-MU SP-12 tom preset, SSM2044 dynamic VCF
/ Tuned lower: ~180Hz
/ 400ms
N: 17640
T: !N
E: e(T*(0-6.9%N))

F: 180*(6.28318%44100)
P: +\(N#F)
S: s P

O: e(T*(0-200%N))
I: 0.4 f S
J: 0.06 f S
D: (O*I)+((1-O)*J)
Q: 4096 v D

V: e(T*(0-300%N))
R: r T
C: 0.2 f R
K: V*C

W: w (E*Q*.85)+(K*.25)
