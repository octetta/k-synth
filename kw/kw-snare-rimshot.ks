/ Kraftwerk Rimshot — Maestro Rhythm-King style
/ Trans-Europe Express, The Model — the clean precise click
/ Very short, tonal, minimal noise
/ 80ms
N: 3528
T: !N
E: e(T*(0-6.9%N))

/ High triangle tone ~800Hz — the Rhythm-King rimshot was tonal
F: 800*(6.28318%44100)
G: 1200*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
A: 1 0 -.111 0 0.04
/ triangle spectrum on each
O: P $ A
I: Q $ A
S: (O+I*.5)

/ Minimal noise — just the click
V: e(T*(0-300%N))
R: r T
K: V*R

W: w E*(S*.7+K*.4)
