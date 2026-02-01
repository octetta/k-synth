/ --- 1. The Drums (Z) ---
N:5512
L:!N
/ E: Decay, K: Kick, S: Snare, H: Hat
E:e -6 * L % N
K:E * s L * 0.02 + 0.1 * e -10 * L % N
S:E * (0.3,1.5) f r N
F:e -25 * L % N
H:0.2 * F * (0.7,1.0) f r N
/ 8-step sequence
Z:K,H,S,H,K,H,S,H

/ --- 2. The Sidechained Bass (S) ---
/ T: Total length of pattern Z
T:44096
I:!T
/ W: Sawtooth, C: Cutoff, B: Bass
W:((I ! 400) * 0.005) - 0.25
C:0.15 + 0.1 * s I % 10000
B:(C,3.5) f W
/ D: Ducking Envelope (goes to 0 at every Kick/Snare hit)
D:1.0 - e -10 * I % 5512
S:D * B

/ --- 3. The Master Mix (M) ---
/ We sum them and scale to 0.5 to keep it clean
M:0.5 * Z + S

/ --- 4. Final Playback ---
\p M
