N:44100
L:!N
/ M: Sawtooth
M:((L ! 400) * 0.01) - 0.5
/ C: Modulated Cutoff
C:0.15 + 0.1 * s L % 5000
/ R: High Resonance
R:N # 3.5
/ G: The Gate (Sine-based)
G:_ 0.5 + s L % 2000
/ X: Filtered and Gated Signal
X:G * (C,R) f M
/ Z: Apply 1500 sample delay to the result
/ In your ksynth.c, 'y' adds 40% of the delayed signal back
Z:1500 y X
\p Z
