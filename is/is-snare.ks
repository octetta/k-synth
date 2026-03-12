/ InSoc Snare — E-MU SP-12 "Snare" preset
/ "What's On Your Mind", "Repetition", "Land of the Blind"
/ SP-12 snare: 12-bit sampled acoustic snare at 26kHz
/ Static Chebyshev filter on snare channel
/ Character: punchy, slightly gritty, not clean — the lo-fi sample texture
/ Very different from clean DM Synclavier snare
/ 200ms
N: 8820
T: !N

/ Pitched body: acoustic snare resonance ~180Hz
B: e(T*(0-35%N))
F: 180*(6.28318%44100)
G: 225*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
O: s P
I: s Q
S: B*((O+I)*.5)

/ 12-bit quantize the body — the SP-12 sample grit
Y: 4096 v S

/ Noise: SP-12 snare had significant noise component
/ Static filter on snare channel — mid-low LP preserves the "thwack"
E: e(T*(0-6.9%N))
R: r T
/ SP-12 snare channel Chebyshev ~3kHz cutoff (static)
L: 0.4 f R
U: E*L

/ Snap transient — the beater crack in the sample
V: e(T*(0-300%N))
K: r T
/ Unfiltered crack (above Chebyshev range = the SP-12 imaging artefact)
A: V*K

/ Quantize the full mix for 12-bit character
Z: (Y*.5)+(U*.6)+(A*.3)
W: w 4096 v Z
