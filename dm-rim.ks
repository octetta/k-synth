/ DM Rimshot — clean and tonal, not harsh
/ The Drumulator/Synclavier rim: very precise, tonal, musical
/ Character: clean high tone, brief, controlled
/ 100ms
N: 4410
T: !N
E: e(T*(0-6.9%N))

/ Two tones: 1600Hz + 2200Hz — slightly inharmonic pair
F: 1600*(6.28318%44100)
G: 2200*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
O: (s P)
I: (s Q)

/ Brief noise — just the attack click
V: e(T*(0-250%N))
R: r T
L: 0.3 f R
A: V*(R-L)

W: w E*((O*.6+I*.4)+(A*.3))
