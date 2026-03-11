/ DM Snare — "metal object hit" style (Synclavier sampled percussion)
/ Construction Time Again / Some Great Reward character
/ Wilder: "recording band members hitting metal pipes and other industrial objects"
/ Character: sharp tonal attack, inharmonic ring, clean noise tail, NO distortion
/ The tonal component IS the snare — not a drum, a metal thing
/ 300ms
N: 13230
T: !N

/ Metal pipe resonance: two inharmonic modes
/ Real metal objects have inharmonic partial ratios
/ Mode 1: ~900Hz (bright ping)
/ Mode 2: ~1340Hz (inharmonic upper partial, ratio ~1.49)
F: 900*(6.28318%44100)
G: 1340*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
O: (s P)*e(T*(0-25%N))
I: (s Q)*e(T*(0-35%N))

/ Third inharmonic mode — makes it feel like a real metal object
J: 2100*(6.28318%44100)
R: +\(N#J)
B: (s R)*e(T*(0-50%N))

/ Noise tail — the "snare rattle" element, clean
E: e(T*(0-6.9%N))
U: r T
Z: E*U

/ Transient click at moment of strike
V: e(T*(0-300%N))
K: r T
A: V*K

W: w (O*.7)+(I*.5)+(B*.3)+(Z*.4)+(A*.3)
