/ DM Clap — clinical, precise, NOT distorted
/ DM claps are clean and forward — the Synclavier's precise sequencing
/ gave them a machine-exact quality. Tuned, almost tonal.
/ 180ms
N: 7938
T: !N

/ DM claps have a slight tonal quality — not pure noise
/ Two noise bursts, but CLEAN (no d() saturation)
R: r T
A: 0.4 f R
B: 0.08 f R
C: A-B
X: w (T*e(T*(0-25%N)))

J: r T
D: 0.4 f J
F: 0.08 f J
G: D-F
Y: w (T*e(T*(0-10%N)))

/ Noise tail — decaying room
E: e(T*(0-6.9%N))
K: r T
/ Slight bandpass for the "tuned" quality
L: 0.3 f K
U: K-(0.08 f K)

W: w (X*C*.5)+(Y*G*.8)+(E*U*.2)
