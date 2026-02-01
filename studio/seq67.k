N:22050
L:!N
/ M: Sawtooth Oscillator
M:((L ! 400) * 0.01) - 0.5
/ C: Constant Cutoff
C:N # 0.15

/ --- Segment X: Low Resonance (Fade Out) ---
R:N # 0.5
/ O: Fade envelope (1 to 0)
O:1 - L % N
X:O * (C,R) f M

/ --- Segment Y: High Resonance (Fade In) ---
S:N # 3.7
/ I: Fade envelope (0 to 1)
I:L % N
Y:I * (C,S) f M

/ --- Join and Play ---
/ We join X and Y. Because of the fades, the junction is seamless.
Z:X,Y
\p Z
