N:44100
L:!N
/ M: Sawtooth Source
M:((L ! 400) * 0.01) - 0.5
/ C: Main Cutoff, R: Resonance, G: Gate
C:0.15 + 0.1 * s L % 5000
R:N # 3.5
G:_ 0.5 + s L % 2000
/ X: The "Dry" Gated Sound
X:G * (C,R) f M

/ --- Left Channel Logic ---
/ Short delay (1000), Darker shimmer (S)
S:0.1 + 0.1 * s L % 1200
A:X + (S,R) f 1000 y X

/ --- Right Channel Logic ---
/ Long delay (3000), Brighter shimmer (T)
T:0.4 + 0.2 * s L % 800
B:X + (T,R) f 3000 y X

/ --- Join ---
/ Final output: 1s Left, then 1s Right
Z:A,B
Q:0.5 * Z
\p Q
