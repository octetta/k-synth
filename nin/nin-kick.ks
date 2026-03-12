/ NIN Kick — "Closer" style
/ Deep sub, slow sweep, tape-saturated. Iggy Pop's Nightclubbing kick had
/ "noise and hiss" — we keep grit by hard-clipping the body and adding
/ broadband noise floor underneath it.
/
/ 400ms — longer sustain than 808, the sub hangs
N: 17640
T: !N

/ Pitch sweep: starts high (big thwack), drops to deep sub
/ 120Hz -> 38Hz over 80ms then sustains
E: e(T*(0-6.9%N))
F: 38+82*e(T*(0-40%N))
D: F*(6.28318%44100)
P: +\D
S: s P

/ Hard clip the sine — tape saturation / SSL desk grit
/ tanh at high gain = soft saturation. Scale up then clip back down.
C: d(S*3)

/ Sub reinforcement: pure low-end sine at sustained pitch, slower decay
G: 38*(6.28318%44100)
Q: +\(N#G)
B: (s Q)*e(T*(0-5%N))

/ Noise floor — grit underneath everything
R: r T
L: 0.06 f R
/ Gate noise with kick envelope — it breathes with the hit
X: E*L*.15

/ Click transient — the beater attack
V: e(T*(0-400%N))
K: r T
A: V*K*.3

W: w (C*E*.9)+(B*.4)+(X*.2)+(A*.2)
