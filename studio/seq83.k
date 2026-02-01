/ --- 1. The Gritty Source (G) ---
N:5512
L:!N
E:e -6 * L % N
K:E * s L * 0.02 + 0.1 * e -10 * L % N
S:E * (0.3,1.5) f r N
H:0.2 * (e -25 * L % N) * (0.8,1.0) f r N
Z:K,H,S,H,K,H,S,H
T:44096
I:!T
W:((I ! 400) * 0.005) - 0.25
C:0.15 + 0.1 * s I % 10000
B:(C,3.5) f W
D:1.0 - e -10 * I % 5512
M:0.5 * Z + D * B
G:h 2.0 * M

/ --- 2. The Phaser Logic (P) ---
/ We take a 10-sample delay of G
X:10 y G
/ We create a 'wobble' (W) using a Sine wave to mix the delay in and out
W:0.5 + 0.5 * s I % 8000
/ P: Mix the original G with the modulated delay X
P:G + W * X

/ --- 3. Final Playback ---
\p P
