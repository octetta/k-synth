/ Wave 1: Pure Sine
P: ~4096
W: s P

/ Wave 2: Sawtooth (1/h)
P: ~4096
H: !32
H: H+1
A: 1%H
W: w P $ A

/ Wave 3: Square (odd harmonics)
P: ~4096
H: !32
H: H+1
M: a(s(H*1.5707963))
A: (1%H)*M
W: w P $ A

/ Wave 4: Triangle (odd harmonics, 1/h^2)
P: ~4096
H: !32
H: H+1
M: a(s(H*1.5707963))
A: (1%(H*H))*M
W: w P $ A

/ Wave 5: Soft Saw (1/h^1.5 rolloff)
P: ~4096
H: !32
H: H+1
A: 1%(H^1.5)
W: w P $ A

/ Wave 6: Bright Saw (flat below h=16, 1/h rolloff above)
P: ~4096
H: !32
H: H+1
C: 16
A: (H<C)+(1-(H<C))*(C%H)
W: w P $ A

/ Wave 7: Thin Pulse (~1/4 duty)
P: ~4096
H: !32
H: H+1
A: (1%H)*a(s(H*0.785398))
W: w P $ A

/ Wave 8: DW-8000 style, cutoff h=4
P: ~4096
H: !32
H: H+1
C: 4
A: (H<C)+(1-(H<C))*(C%H)
W: w P $ A

/ Wave 9: DW-8000 style, cutoff h=8
P: ~4096
H: !32
H: H+1
C: 8
A: (H<C)+(1-(H<C))*(C%H)
W: w P $ A

/ Wave 10: DW-8000 style, cutoff h=12
P: ~4096
H: !32
H: H+1
C: 12
A: (H<C)+(1-(H<C))*(C%H)
W: w P $ A

/ Wave 11: DW-8000 style, cutoff h=24
P: ~4096
H: !32
H: H+1
C: 24
A: (H<C)+(1-(H<C))*(C%H)
W: w P $ A

/ Wave 12: PPG soft (cutoff h=3)
P: ~4096
H: !32
H: H+1
C: 3
A: (H<C)+(1-(H<C))*(C%H)
W: w P $ A

/ Wave 13: Vowel AA (formant h=3)
P: ~4096
H: !32
H: H+1
D: H-3
E: e(0-D*D*.3)
A: (1%H)*E
W: w P $ A

/ Wave 14: Vowel AE (formant h=5)
P: ~4096
H: !32
H: H+1
D: H-5
E: e(0-D*D*.25)
A: (1%H)*E
W: w P $ A

/ Wave 15: Vowel EH (formant h=7)
P: ~4096
H: !32
H: H+1
D: H-7
E: e(0-D*D*.2)
A: (1%H)*E
W: w P $ A

/ Wave 16: Vowel EE (formant h=9)
P: ~4096
H: !32
H: H+1
D: H-9
E: e(0-D*D*.2)
A: (1%H)*E
W: w P $ A

/ Wave 17: Vowel IH (formant h=11)
P: ~4096
H: !32
H: H+1
D: H-11
E: e(0-D*D*.18)
A: (1%H)*E
W: w P $ A

/ Wave 18: Vowel OH (formant h=4)
P: ~4096
H: !32
H: H+1
D: H-4
E: e(0-D*D*.22)
A: (1%H)*E
W: w P $ A

/ Wave 19: Vowel OO (formant h=2)
P: ~4096
H: !32
H: H+1
D: H-2
E: e(0-D*D*.35)
A: (1%H)*E
W: w P $ A

/ Wave 20: Vowel UH (dual formant h=3 and h=8)
P: ~4096
H: !32
H: H+1
D: H-3
B: H-8
E: e(0-D*D*.3)+e(0-B*B*.25)
A: (1%H)*E
W: w P $ A

/ Wave 21: Bell (stretched inharmonic partials, 1/h^2)
/ uses o with explicit stretched harmonic numbers
P: ~4096
H: !32
H: H+1
R: H+H*.04
A: 1%(H*H)
W: w P o (R*A)

/ Wave 22: Hollow (even harmonics only)
P: ~4096
H: !32
H: H+1
M: a(c(H*1.5707963))
A: (1%H)*M
W: w P $ A

/ Wave 23: Reed (odd harmonics, fast 1/h^3 rolloff)
P: ~4096
H: !32
H: H+1
M: a(s(H*1.5707963))
A: (1%(H*H*H))*M
W: w P $ A

/ Wave 24: Nasal (sawtooth with notch at h=4)
P: ~4096
H: !32
H: H+1
D: H-4
N: e(0-D*D*.8)
A: (1%H)*(1-N)
W: w P $ A

/ Wave 25: Pluck (1/h with upper partial boost at h=20)
P: ~4096
H: !32
H: H+1
D: H-20
B: e(0-D*D*.05)
A: (1%H)+B*.5
W: w P $ A

/ Wave 26: Clavinet (fast rolloff 1/h^1.8, notch at h=6)
P: ~4096
H: !32
H: H+1
D: H-6
N: e(0-D*D*1.5)
A: (1%(H^1.8))*(1-N)
W: w P $ A

/ Wave 27: Organ 1 (drawbar: h1 + h2 + h4)
P: ~4096
A: 1.0 0.7 0 0.5
W: w P $ A

/ Wave 28: Organ 2 (drawbar: h1+h2+h4+h8+h16)
P: ~4096
A: 1.0 0.8 0 0.6 0 0 0 0.4 0 0 0 0 0 0 0 0.2
W: w P $ A

/ Wave 29: Soft Pad (wide Gaussian peak at h=5)
P: ~4096
H: !32
H: H+1
D: H-5
E: e(0-D*D*.08)
A: E
W: w P $ A

/ Wave 30: Bright Pad (sawtooth base plus narrow boost at h=12)
P: ~4096
H: !32
H: H+1
D: H-12
E: e(0-D*D*.15)
A: (1%H)+E
W: w P $ A

/ Wave 31: ESQ-1 style (two spectral bumps at h=4 and h=14)
P: ~4096
H: !32
H: H+1
D: H-4
B: H-14
E: e(0-D*D*.2)+e(0-B*B*.25)*.6
A: (1%H)*E
W: w P $ A

/ Wave 32: Inharmonic Cluster (partial stretch via sine)
/ uses o with explicit stretched harmonic numbers
P: ~4096
H: !32
H: H+1
R: H+s(H*.7)*.3
A: 1%H
W: w P o (R*A)
