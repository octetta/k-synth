/ InSoc Electronic Snare — E-MU SP-12 "Electronic Snare" preset
/ SP-12 had TWO electronic snare presets alongside the acoustic one
/ The electronic snare was more synthesised-sounding — used on "Running",
/ accent hits in "What's On Your Mind" verses
/ Character: punchy synthetic crack, less acoustic than main snare
/ 160ms
N: 7056
T: !N

/ Electronic body: higher pitched tone, fast decay — the "snap"
B: e(T*(0-50%N))
F: 280*(6.28318%44100)
P: +\(N#F)
O: s P
S: B*O

/ 12-bit quantize
Y: 4096 v S

/ Noise: filtered brighter than acoustic snare (electronic = harsher)
E: e(T*(0-6.9%N))
R: r T
/ Electronic snare: less LP filtering than acoustic (brighter channel)
L: 0.5 f R
U: E*L

V: e(T*(0-400%N))
K: r T
A: V*K

Z: (Y*.4)+(U*.7)+(A*.4)
W: w 4096 v Z
