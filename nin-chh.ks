/ NIN Closed Hi-Hat — industrial metallic, compressed tight
/ Brighter and harsher than 808 — more white noise + 1-bit
/ NIN hats have an almost distorted quality, no clean hi-hat ring
/ 120ms
N: 5292
T: !N
E: e(T*(0-6.9%N))

/ Very fast extra decay for the "choked" industrial feel
C: e(T*(0-20%N))

/ 1-bit noise — metallic core
M: m T
/ White noise with HP for the air
R: r T
L: 0.7 f R
H: R-L

/ distort (clip) the mix — industrial crunch
Z: d((M*.6+H*.5)*2)

W: w E*C*Z
