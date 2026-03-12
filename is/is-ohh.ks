/ InSoc Open Hi-Hat — E-MU SP-12 preset
/ Same unfiltered character as CHH but longer sustain
/ 300ms
N: 13230
T: !N
E: e(T*(0-6.9%N))

R: r T
M: m T
Z: M*.6+R*.4
Q: 4096 v Z

/ Slight longer ring — the open hat sustain
F: e(T*(0-3%N))

W: w (E*Q*.8)+(F*M*.2)
