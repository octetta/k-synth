/ --- 1. The Source Mix (M) ---
/ (Including Drums, Arp, Sidechain, and Grit from previous steps)
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
M:h 1.5 * 0.5 * Z + (1.0 - e -10 * I % 5512) * B

/ --- 2. The Reverb / Diffusion Bank ---
/ We create 3 different delay taps with prime-ish offsets
/ D1: 1103 samples (~50ms)
/ D2: 1733 samples (~78ms)
/ D3: 2591 samples (~117ms)
D1:1103 y M
D2:1733 y M
D3:2591 y M

/ --- 3. The Final Wash (R) ---
/ We mix the original 'Dry' signal M with a 'Wet' blend of the delays
/ We scale the delays down (0.1) so they don't wash out the kick
R:M + 0.1 * D1 + D2 + D3

/ --- 4. Final Playback ---
\p R
