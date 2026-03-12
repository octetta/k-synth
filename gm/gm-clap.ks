/ Moroder Clap — Moog modular
/ Not confirmed specifically on IFL — the clap was more common in "From Here to Eternity"
/ Same architecture: white noise + multi-burst envelope from Moog VCA
/ The Moog clap = staggered noise bursts through the VCA envelope
/ 120ms
N: 5292
T: !N

R: r T

/ Three staggered noise bursts — simulating multiple hands
A: e(T*(0-30%N))
J: r T
B: e(T*(0-18%N))
K: r T
/ Moog LP on the clap: moderate brightness
L: 0.4 f R
M: 0.4 f J
X: 0.4 f K

/ The Moog VCA opening and closing rapidly = the clap character
P: (T*A*L*.4)+(T*B*M*.7)+(T*e(T*(0-8%N))*X)

W: w P
