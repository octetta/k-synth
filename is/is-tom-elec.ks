/ InSoc Electronic Tom — E-MU SP-12 "Electronic Tom" preset
/ SP-12 had FOUR electronic tom presets alongside the acoustic ones
/ Electronic toms: synthesised tones at 12-bit, more "clinical" than acoustic
/ Used for syncopated accent patterns — the SP-12 electronic-ness fully showing
/ Tuned mid: ~220Hz, faster sweep
/ 300ms
N: 13230
T: !N
E: e(T*(0-6.9%N))

/ Electronic tom: sharper pitch sweep (synthetic character)
F: 80+140*e(T*(0-120%N))
D: F*(6.28318%44100)
P: +\D
S: s P

/ Dynamic VCF — same SSM2044 architecture
O: e(T*(0-200%N))
I: 0.5 f S
J: 0.06 f S
B: (O*I)+((1-O)*J)
Q: 4096 v B

V: e(T*(0-400%N))
R: r T
C: 0.2 f R
K: V*C

W: w (E*Q*.85)+(K*.2)
