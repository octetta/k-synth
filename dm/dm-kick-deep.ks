/ DM Kick — Construction Time Again / EMU Drumulator + ARP 2600 style
/ Pure synthesised sub — the band explicitly layered synth bass under samples
/ More sustained sub, slower decay, very clean
/ 500ms
N: 22050
T: !N
E: e(T*(0-6.9%N))

/ ARP 2600 kick character: starts around 100Hz, settles to ~40Hz
/ Longer sweep than stomp kick
F: 40+65*e(T*(0-50%N))
D: F*(6.28318%44100)
P: +\D
O: s P

/ Pure sub reinforcement at fundamental — the EMU Drumulator "fattening" trick
G: 38*(6.28318%44100)
Q: +\(N#G)
I: (s Q)*e(T*(0-4%N))

/ Very short attack transient — synthesised, NOT noisy
/ A tight sine burst at ~200Hz simulates the EMU Drumulator click
J: 200*(6.28318%44100)
R: +\(N#J)
B: (s R)*e(T*(0-600%N))

W: w (E*O*.9)+(I*.3)+(B*.2)
