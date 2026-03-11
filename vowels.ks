/ Vowel and Formant Wavetables
/ All use sawtooth base (1/h) shaped by Gaussian formant envelopes.
/ Single formant: one resonant peak. Dual/triple: multiple peaks.
/ Formant center F, width controlled by gaussian sigma W (larger = narrower).

/ ===== Single Formant Vowels =====

/ Wave 1: OO (as in "boot") — very dark, formant at h=2
P: ~4096
H: !32
H: H+1
D: H-2
A: (1%H)*e(0-D*D*.4)
W: w P $ A

/ Wave 2: OH (as in "boat") — dark, formant at h=4
P: ~4096
H: !32
H: H+1
D: H-4
A: (1%H)*e(0-D*D*.25)
W: w P $ A

/ Wave 3: AA (as in "father") — open, formant at h=3
P: ~4096
H: !32
H: H+1
D: H-3
A: (1%H)*e(0-D*D*.3)
W: w P $ A

/ Wave 4: AE (as in "cat") — bright open, formant at h=5
P: ~4096
H: !32
H: H+1
D: H-5
A: (1%H)*e(0-D*D*.25)
W: w P $ A

/ Wave 5: EH (as in "bed") — mid, formant at h=7
P: ~4096
H: !32
H: H+1
D: H-7
A: (1%H)*e(0-D*D*.2)
W: w P $ A

/ Wave 6: IH (as in "bit") — upper mid, formant at h=10
P: ~4096
H: !32
H: H+1
D: H-10
A: (1%H)*e(0-D*D*.18)
W: w P $ A

/ Wave 7: EE (as in "beat") — bright, formant at h=13
P: ~4096
H: !32
H: H+1
D: H-13
A: (1%H)*e(0-D*D*.15)
W: w P $ A

/ Wave 8: Nasal NG — notch at h=4, peak at h=7
P: ~4096
H: !32
H: H+1
D: H-7
N: H-4
A: (1%H)*e(0-D*D*.2)*(1-e(0-N*N*.8))
W: w P $ A

/ ===== Dual Formant (more realistic) =====

/ Wave 9: UH (as in "but") — F1=h3, F2=h8
P: ~4096
H: !32
H: H+1
D: H-3
B: H-8
A: (1%H)*(e(0-D*D*.3)+e(0-B*B*.25))
W: w P $ A

/ Wave 10: AH (as in "palm") — F1=h4, F2=h12
P: ~4096
H: !32
H: H+1
D: H-4
B: H-12
A: (1%H)*(e(0-D*D*.25)+e(0-B*B*.2)*.6)
W: w P $ A

/ Wave 11: AW (as in "law") — F1=h3, F2=h6
P: ~4096
H: !32
H: H+1
D: H-3
B: H-6
A: (1%H)*(e(0-D*D*.3)+e(0-B*B*.3)*.7)
W: w P $ A

/ Wave 12: EW (as in "few") — F1=h2, F2=h14
P: ~4096
H: !32
H: H+1
D: H-2
B: H-14
A: (1%H)*(e(0-D*D*.4)+e(0-B*B*.15)*.8)
W: w P $ A

/ Wave 13: OI (as in "boy") — F1=h5, F2=h10
P: ~4096
H: !32
H: H+1
D: H-5
B: H-10
A: (1%H)*(e(0-D*D*.2)+e(0-B*B*.2)*.7)
W: w P $ A

/ Wave 14: Choir (three formants: h3, h8, h16)
P: ~4096
H: !32
H: H+1
D: H-3
B: H-8
C: H-16
A: (1%H)*(e(0-D*D*.3)+e(0-B*B*.2)*.8+e(0-C*C*.15)*.5)
W: w P $ A

/ Wave 15: Whisper (broad noise-like formant, center h=6, very wide)
P: ~4096
H: !32
H: H+1
D: H-6
A: (1%H)*e(0-D*D*.03)
W: w P $ A

/ Wave 16: Throat (low formant h=2, strong upper h=18)
P: ~4096
H: !32
H: H+1
D: H-2
B: H-18
A: (1%H)*(e(0-D*D*.5)+e(0-B*B*.12)*.9)
W: w P $ A
