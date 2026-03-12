/ Kraftwerk Clap — Mattel Synsonics / Simmons Clap Trap style
/ Very synthetic, NOT organic — the machine-generated clap
/ "Computer World" era: two fast noise bursts, very dry, very precise
/ Character: clinical, metallic edge, no room at all
/ 150ms
N: 6615
T: !N

/ Burst 1 — very fast
R: r T
A: 0.3 f R
B: 0.06 f R
C: A-B
X: w (T*e(T*(0-35%N)))

/ Burst 2 — slightly longer, same character
J: r T
D: 0.3 f J
F: 0.06 f J
G: D-F
Y: w (T*e(T*(0-12%N)))

/ Synthanorma clip on both bursts — the machine precision crispness
P: d(X*C*3)
Q: d(Y*G*3)

/ Tail — very short, just the room-free reverb tail
E: e(T*(0-6.9%N))
K: r T
L: 0.2 f K
U: E*L*.15

W: w (P*.4)+(Q*.8)+U
