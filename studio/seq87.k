/ --- 1. The Core Mix (M) ---
N:5512
L:!N
E:e -6 * L % N
K:E * s L * 0.02 + 0.1 * e -10 * L % N
S:E * (0.3,1.5) f r N
H:0.2 * (e -25 * L % N) * (0.8,1.0) f r N
Z:K,H,S,H,K,H,S,H

T:44096
I:!T
P:0.005 + 0.002 * (_ I % 5512) ! 3
W:((I * P + 0.0002 * s I % 3000) ! 1) - 0.5
B:(0.15 + 0.1 * s I % 10000, 3.5) f W
M:h 1.2 * 0.5 * Z + (1.0 - e -10 * I % 5512) * B

/ --- 2. The Lo-Fi Bitcrush (Q) ---
/ We multiply by 10, floor it, then divide by 10.
/ This limits the precision of the waveform, creating a 4-bit 'Crunch'.
Q:(_ 10 * M) * 0.1

/ --- 3. The Efficient Space (R) ---
/ Instead of 3 delays, we use 1 long delay (4000 samples)
/ and mix it 30% with the crushed signal.
R:0.7 * Q + 0.3 * 4000 y Q

\p R
