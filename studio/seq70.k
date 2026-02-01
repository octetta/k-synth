N:44100
L:!N
/ M: Sawtooth
M:((L ! 400) * 0.01) - 0.5

/ C: Modulated Cutoff (The "Liquid" part)
C:0.15 + 0.1 * s L % 5000

/ R: High Resonance
R:N # 3.5

/ G: The Gate (The "Rhythmic" part)
/ We take a Sine wave, add 0.5, and Floor it. 
/ This creates a pulse that is '1' for half the cycle and '0' for the other.
G:_ 0.5 + s L % 2000

/ X: Filtered Signal
X:(C,R) f M

/ Z: Gated Output
Z:G * X
\p Z
