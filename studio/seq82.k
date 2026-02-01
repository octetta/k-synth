/ --- 1. The Clean Mix (M) ---
/ (Using the logic from the previous step)
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

/ --- 2. The Grit Processing (G) ---
/ We multiply M by 2.0 to 'push' it into the distortion circuit,
/ then apply h (Tanh) to squash it back down.
G:h 2.0 * M

/ --- 3. Final Playback ---
\p G
