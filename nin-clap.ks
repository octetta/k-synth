/ NIN Clap — smashed, distorted, not a clean 808 clap
/ More like a gunshot clap — heavy compression and saturation
/ Three noise bursts BUT each is distorted and has mid content
/ 200ms
N: 8820
T: !N

/ Burst 1 — early, harsh, mid-heavy (2-4kHz band)
R: r T
A: 0.5 f R
B: 0.1 f R
C: A-B
/ distort each burst
X: w (T*e(T*(0-30%N)))
P: d(X*C*4)

/ Burst 2 — slightly later, similar character
J: r T
D: 0.5 f J
F: 0.1 f J
G: D-F
Y: w (T*e(T*(0-15%N)))
Q: d(Y*G*4)

/ Burst 3 — main body, loudest, pushed through heavy saturation
K: r T
E: K-(0.114 f K)
H: 0.256 f E
Z: w (T*e(T*(0-6%N)))
/ clip harder for the main burst
S: d(Z*H*6)

/ Noise bed underneath — keeps it from being too clean
U: r T
V: e(T*(0-6.9%N))
L: 0.3 f U

W: w (P*.25)+(Q*.4)+S+(V*L*.2)
