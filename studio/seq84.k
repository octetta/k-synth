/ --- 1. The Drums (Z) ---
N:5512
L:!N
E:e -6 * L % N
K:E * s L * 0.02 + 0.1 * e -10 * L % N
S:E * (0.3,1.5) f r N
H:0.2 * (e -25 * L % N) * (0.8,1.0) f r N
Z:K,H,S,H,K,H,S,H

/ --- 2. The Arpeggiated Bass (B) ---
T:44096
I:!T
/ J: The Step Counter (0, 1, 2, 3...) jumping every beat
J:_ I % 5512
/ P: The Pitch Map. We use the Rand (r) verb or math to shift.
/ This formula makes the pitch jump between 3 notes.
P:0.005 + 0.002 * J ! 3
/ W: The Arpeggiated Sawtooth
W:((I * P) ! 1) - 0.5

/ --- 3. Filter & Sidechain ---
C:0.15 + 0.1 * s I % 10000
B:(C,3.5) f W
D:1.0 - e -10 * I % 5512
S:D * B

/ --- 4. Master Mix (M) ---
M:h 1.5 * 0.5 * Z + S
\p M
