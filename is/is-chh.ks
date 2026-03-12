/ InSoc Closed Hi-Hat — E-MU SP-12 preset
/ SP-12 hi-hat channels were UNFILTERED — no SSM2044 on hats
/ "Reconstruction filter deliberately omitted: brighter sound due to imaging"
/ The hi-hat imaging above 13kHz gives it its distinctive metallic brightness
/ That brightness-beyond-Nyquist quality = very "digital metallic" texture
/ 80ms
N: 3528
T: !N
E: e(T*(0-6.9%N))

/ Unfiltered approach: noise with NO LP filtering — all content passes
/ This simulates the SP-12's no-reconstruction-filter character
R: r T
/ 1-bit noise: the inharmonic metallic hash — simulates imaging artefacts
M: m T

/ Fast tight double-decay for the SP-12 hat character
/ SP-12 had very tight, fast hat samples
C: e(T*(0-25%N))

/ The "digital metallic" texture: quantize to 12-bit
Z: M*.6+R*.4
Q: 4096 v Z

W: w C*E*Q
