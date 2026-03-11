/ Percussion Wavetables
/ Wavetables useful as the tonal/noise component of drums and percussion.
/ These are single-cycle snapshots — envelope shaping is done outside ksynth.
/ Many use inharmonic partials via explicit o with stretched frequencies.

/ ===== Drums =====

/ Wave 1: Kick Body — sine with strong sub
/ Low harmonics only, h2 boosted relative to h1
P: ~4096
A: 1.0 0.5 0.1
W: w P $ A

/ Wave 2: Kick Click — dense upper harmonics, thin fundamental
/ Models the attack transient layer of a kick
P: ~4096
H: !32
H: H+1
D: H-16
A: (1%H)*e(0-D*D*.03)
W: w P $ A

/ Wave 3: Snare Tone — sawtooth with notch at h=4 (nasal body)
P: ~4096
H: !32
H: H+1
D: H-4
N: e(0-D*D*.5)
A: (1%H)*(1-N)
W: w P $ A

/ Wave 4: Snare Wire — dense inharmonic, many equal partials
/ Models the snare wire rattle component
P: ~4096
H: !32
H: H+1
R: H+s(H*1.3)*.4
A: H*0+1
W: w P o (R*A)

/ Wave 5: Tom Body — smooth sine-like with h2 and h3 present
P: ~4096
A: 1.0 0.35 0.12 0.04
W: w P $ A

/ Wave 6: Rimshot — bright attack, peak near h=10
P: ~4096
H: !32
H: H+1
D: H-10
A: (1%H)+e(0-D*D*.12)*2
W: w P $ A

/ ===== Cymbals and Metal =====

/ Wave 7: Hi-Hat Body — inharmonic stretch, fast upper rolloff
/ Partial ratios based on circular plate modes: 1, 2.76, 5.4, 8.93...
P: ~4096
R: 1 2.76 5.4 8.93 13.3 18.6 24.8
A: 1 0.6 0.4 0.3 0.2 0.14 0.1
W: w P o (R*A)

/ Wave 8: Cymbal Wash — dense inharmonic cloud
P: ~4096
H: !24
H: H+1
R: H+s(H*0.9)*.6
A: 1%H
W: w P o (R*A)

/ Wave 9: Bell Strike — sparse inharmonic (tubular bell partials)
/ Ratios: 1, 2.756, 5.404, 8.933, 13.34
P: ~4096
R: 1 2.756 5.404 8.933 13.34
A: 1 0.7 0.5 0.3 0.15
W: w P o R

/ Wave 10: Cowbell — two inharmonic clusters
/ 800Hz fundamental, strong partial near 1.5x and 2.4x
P: ~4096
R: 1 1.5 2.4 3.6 4.9
A: 1 0.8 0.6 0.3 0.15
W: w P o (R*A)

/ Wave 11: Triangle — pure high inharmonic, slow decay
P: ~4096
R: 1 2.9 5.8 9.6
A: 1 0.4 0.2 0.1
W: w P o (R*A)

/ Wave 12: Clave / Wood Block — hard attack, fast upper rolloff
P: ~4096
H: !16
H: H+1
R: H+s(H*.4)*.2
A: 1%(H^1.4)
W: w P o (R*A)

/ ===== Noise and Texture =====

/ Wave 13: White-ish noise (pseudo-random via index hash)
/ Use as noise layer in drum synthesis
P: ~4096
W: m P

/ Wave 14: Buzz Noise (6 slightly detuned sines)
P: ~4096
W: b P

/ Wave 15: Low Noise — noise shaped to low frequencies
/ Broad Gaussian peak at h=3 over 1/h base
P: ~4096
H: !32
H: H+1
D: H-3
A: (1%H)*e(0-D*D*.04)
W: w P $ A

/ Wave 16: Band Noise — mid-frequency texture
/ Gaussian peak at h=8 over flat base
P: ~4096
H: !32
H: H+1
D: H-8
A: e(0-D*D*.05)
W: w P $ A

/ ===== Melodic Percussion =====

/ Wave 17: Marimba — odd harmonics, very fast rolloff, warm
P: ~4096
H: !16
H: H+1
M: a(s(H*1.5707963))
A: (1%(H*H*H))*M
W: w P $ A

/ Wave 18: Vibraphone — strong h1 and h2, slight inharmonicity
P: ~4096
R: 1 2.01 3.0 4.02
A: 1 0.35 0.08 0.03
W: w P o (R*A)

/ Wave 19: Xylophone — bright, fast rolloff, slight stretch
P: ~4096
R: 1 3.1 6.2 10.3
A: 1 0.4 0.15 0.05
W: w P o (R*A)

/ Wave 20: Steel Pan — complex inharmonic cluster
P: ~4096
R: 1 1.98 3.01 3.97 5.02 6.1
A: 1 0.8 0.6 0.4 0.25 0.12
W: w P o (R*A)
