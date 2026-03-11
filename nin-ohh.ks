/ NIN Open Hi-Hat — longer but still harsh and metallic
/ Not clean — gritty, with noise floor
/ 350ms
N: 15435
T: !N
E: e(T*(0-6.9%N))

M: m T
R: r T
L: 0.7 f R
H: R-L
Z: d((M*.6+H*.5)*2)

/ Slower secondary decay for the open ring
F: e(T*(0-4%N))

W: w (E*Z*.7)+(F*M*.3)
