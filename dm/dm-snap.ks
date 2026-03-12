/ DM Snap — Master and Servant whip/snap energy
/ "we tried to sample a real whip but it was hopeless"
/ "Daniel Miller standing in the studio hissing and spitting"
/ The energy: a very sharp, fast, bright transient
/ Used as accent hit in patterns — not a snare, not a clap
/ 80ms
N: 3528
T: !N

/ Very fast broadband burst — the crack/snap
E: e(T*(0-6.9%N))
R: r T

/ HP filtered to be very bright — the "whip air" above 3kHz
L: 0.4 f R
H: R-L

/ Extra bright component: 1-bit filtered bright
M: m T
B: M-(0.5 f M)

/ Tiny tonal stinger — the pitch-specific "crack" frequency
/ A sharp crack has a dominant frequency around 4-6kHz
F: 5000*(6.28318%44100)
P: +\(N#F)
O: (s P)*e(T*(0-60%N))

V: e(T*(0-300%N))

W: w E*(H*.5+B*.3)+(V*O*.3)
