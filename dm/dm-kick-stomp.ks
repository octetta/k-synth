/ DM Kick — Personal Jesus "flight case stomp" style
/ Flood: "recording ourselves kicking a bit of metal in the booth"
/ then room mic layered underneath to thicken it
/ Character: thuddy, LOW, metallic resonance under the thud, NO distortion
/ Clean but massive — the antithesis of the NIN kick
/ 350ms
N: 15435
T: !N
E: e(T*(0-6.9%N))

/ Primary body: deep low frequency, very slight pitch drop (stomp, not sine sweep)
/ ARP 2600-style sub — starts at ~85Hz drops quickly to ~52Hz (the metal resonance)
F: 52+33*e(T*(0-120%N))
D: F*(6.28318%44100)
P: +\D
O: s P

/ Second harmonic reinforcement — the metallic ring of the casing
/ A metal floor case resonates at multiple harmonics, not just fundamental
G: 104*(6.28318%44100)
Q: +\(N#G)
I: (s Q)*e(T*(0-15%N))

/ The "room mic" layer: broadband thud, very lowpass
R: r T
L: 0.06 f R
C: L*e(T*(0-20%N))

/ Transient click — the initial strike impact (very short, broadband)
V: e(T*(0-500%N))
K: r T
A: V*K

W: w (E*O*.9)+(I*.25)+(C*.2)+(A*.15)
