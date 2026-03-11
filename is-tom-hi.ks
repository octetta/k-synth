/ InSoc Hi Tom — E-MU SP-12 "Tom" preset with SSM2044 dynamic VCF
/ SP-12 toms: TWO channels each with SSM2044 dynamic filter
/ "5ms sloping attack followed by a decay" — the filter OPENS then closes
/ This gives the distinctive SP-12 tom: bright attack → dark sustain
/ Also: "way tuned-down SP-12 tom" = tom transposed down for the kick layer
/ Standard tom tuning: ~300Hz
/ 350ms
N: 15435
T: !N
E: e(T*(0-6.9%N))

/ Acoustic tom body: ~300Hz
F: 300*(6.28318%44100)
P: +\(N#F)
S: s P

/ SSM2044 dynamic VCF: 5ms attack then decay
/ At 44100: 5ms = 221 samples
/ Filter starts open (bright = high ct), closes (dark = low ct)
/ Bright ct~0.5 (3509Hz), dark ct~0.08 (561Hz)
/ Blend: bright * short_env + dark * long_env
O: e(T*(0-200%N))
I: 0.5 f S
J: 0.08 f S
/ Dynamic filter: open at attack (O=1), closed as it decays
D: (O*I)+((1-O)*J)

/ 12-bit quantize
Q: 4096 v D

/ Noise transient — the stick hit
V: e(T*(0-300%N))
R: r T
C: 0.3 f R
K: V*C

W: w (E*Q*.85)+(K*.25)
