/ DM Closed Hi-Hat — clean, precise, slightly metallic
/ DM hats are NOT distorted — they're clinical and tight
/ The EMU factory samples had this character: clean metal shimmer
/ 100ms
N: 4410
T: !N
E: e(T*(0-6.9%N))

/ 1-bit noise: metallic character without distortion
M: m T
/ White noise for air — HP to keep it bright
R: r T
L: 0.6 f R
H: R-L

/ Short additional decay multiplier for tight choke
C: e(T*(0-15%N))

W: w C*E*(M*.5+H*.5)
