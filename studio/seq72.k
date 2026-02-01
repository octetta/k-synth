N:44100
L:!N
/ M: Sawtooth
M:((L ! 400) * 0.01) - 0.5

/ C: Primary Liquid Cutoff
C:0.15 + 0.1 * s L % 5000
/ R: Resonance
R:N # 3.5

/ G: The Gate
G:_ 0.5 + s L % 2000

/ X: Filtered and Gated Signal (Dry)
X:G * (C,R) f M

/ S: Shimmer Cutoff (Moves faster than C)
S:0.3 + 0.2 * s L % 1000
/ Y: Delayed signal with a different filter profile
D:(S,R) f 2500 y X

/ Z: Mix original and Shimmering Delay
Z:X + D
\p Z
