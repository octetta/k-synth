/ DM Lo Tom — deep metallic tom, Synclavier-sampled character
/ Not a conventional tom — more like a metal barrel or tank hit
/ Low resonance ~80Hz with inharmonic content, long clean decay
/ 600ms
N: 26460
T: !N
E: e(T*(0-6.9%N))

/ Primary mode: low metallic resonance
F: 78*(6.28318%44100)
P: +\(N#F)
O: (s P)

/ Inharmonic upper partial — gives the "metallic" quality vs clean tom
/ Not 2x fundamental — slightly off
G: 174*(6.28318%44100)
Q: +\(N#G)
I: (s Q)*e(T*(0-10%N))

/ Small pitch drop at attack (the "thud" character)
J: 78+30*e(T*(0-120%N))
D: J*(6.28318%44100)
R: +\D
B: s R

/ Strike transient
V: e(T*(0-250%N))
K: r T
C: 0.1 f K
A: V*C

W: w (E*B*.85)+(I*.3)+(A*.2)
