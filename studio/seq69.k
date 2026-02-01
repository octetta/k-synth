N:44100
L:!N
/ Oscillator: Sawtooth
M:((L ! 400) * 0.01) - 0.5

/ C: Modulated Cutoff (Sine wave oscillates between 0.05 and 0.25)
/ We divide L by 5000 to slow down the wobble
C:0.15 + 0.1 * s L % 5000

/ R: High Resonance
R:N # 3.6

/ G: The Gate (On for 3000 samples, off for 3000)
G:0.5 > (L ! 6000) % 6000

/ Process: Filter then multiply by Gate
Z:G * (C,R) f M
\p Z
