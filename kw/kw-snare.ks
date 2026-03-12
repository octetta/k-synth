/ Kraftwerk Snare — Simmons SDS-V snare module
/ SDS-V snare: triangle VCO (higher pitch than kick) + heavy noise component
/ "Numbers", "Computer World" — the archetypal electronic snare
/ Character: very tonal with lots of noise, fast, precise, no room ambience
/ The SSM2044 filter on noise = more mid-present, not just white
/ 220ms
N: 9702
T: !N

/ Triangle VCO — snare tuned higher than kick, ~220Hz, small pitch bend
F: 220+60*e(T*(0-150%N))
D: F*(6.28318%44100)
P: +\D
A: 1 0 -.111 0 0.04 0 -.0204
S: P $ A
B: e(T*(0-30%N))
/ body with fast decay
O: B*S

/ Heavy noise — SDS-V snare has higher noise level than kick
/ SSM2044 LP filter on noise at ~800Hz gives the characteristic mid-snap
E: e(T*(0-6.9%N))
R: r T
/ LP at ~800Hz: ct=0.114
L: 0.114 f R
/ slight resonance boost on the snare filter
M: 0.114 0.4 f R
/ filtered noise blend
U: E*(L*.6+M*.4)

/ Click transient
V: e(T*(0-400%N))
K: r T
C: V*K

/ Synthanorma clip on body
Q: d(O*2)

W: w (Q*.5)+(U*.6)+(C*.3)
