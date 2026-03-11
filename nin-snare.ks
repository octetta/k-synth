/ NIN Snare — ring-modulated, SSL-distorted, room rattled
/ 280ms
N: 12348
T: !N

/ Body: two detuned resonant modes, fast decay ~15ms
B: e(T*(0-40%N))
F: 185*(6.28318%44100)
G: 230*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
/ parenthesise each sine to prevent FM parsing
O: s P
I: s Q
S: B*((O+I)*.5)

/ Ring modulation carrier ~320Hz
/ Multiplying body by carrier produces sidebands at 185±320, 230±320
/ = 505Hz, 550Hz (sum) and 135Hz, 90Hz (diff) — the metallic brittleness
K: 320*(6.28318%44100)
J: +\(N#K)
M: s J
/ ring modulated body
Y: S*M

/ Hard clip the ring-mod body — SSL desk saturation
D: d(Y*4)

/ Noise tail — full-length room rattle
E: e(T*(0-6.9%N))
R: r T
U: E*R

/ High-mid grit noise: bandpass 1-4kHz, fast decay
V: e(T*(0-25%N))
Z: r T
A: 0.5 f Z
H: A-(0.1 f Z)
X: V*H

W: w (D*.5)+(S*.3)+(U*.5)+(X*.4)
