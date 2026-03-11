/ InSoc SP-12 Percussion accent — the distinctive synth percussion hit
/ SP-12 electronic toms used as accent/stab hits in "Repetition", "Ozar Midrashim"
/ Very short, electronic, quantized — a pure machine accent
/ Tuned high: ~600Hz sweep, 100ms
N: 4410
T: !N
E: e(T*(0-6.9%N))

/ Sharp electronic pitch sweep — more aggressive than tom
F: 120+480*e(T*(0-150%N))
D: F*(6.28318%44100)
P: +\D
S: s P

/ Dynamic VCF — the SSM2044 character
O: e(T*(0-200%N))
I: 0.5 f S
J: 0.04 f S
B: (O*I)+((1-O)*J)

/ 12-bit quantize — this is all SP-12
Q: 4096 v B

V: e(T*(0-600%N))
R: r T
K: V*R

W: w (E*Q*.9)+(K*.2)
