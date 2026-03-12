/ Kraftwerk Hi Tom — Simmons SDS-V "thuuummm" tom
/ "Trans-Europe Express", "The Robots"
/ THE defining Simmons sound: pure triangle VCO, long pitch sweep, minimal noise
/ Tuned high: ~400Hz falling to ~200Hz — the cosmic syn-tom
/ 500ms
N: 22050
T: !N
E: e(T*(0-6.9%N))

/ Triangle VCO: 400Hz -> 200Hz sweep, moderate bend speed
F: 200+200*e(T*(0-40%N))
D: F*(6.28318%44100)
P: +\D
A: 1 0 -.111 0 0.04 0 -.0204
S: P $ A

/ Minimal noise — SDS-V tom had lower noise level than snare
R: r T
L: 0.15 f R
U: e(T*(0-20%N))*L

/ Click
V: e(T*(0-600%N))
K: r T
C: V*K

/ Synthanorma clip — gentle
O: d(S*1.2)

W: w (E*O*.9)+(U*.15)+(C*.2)
