/ DM Snare — "People Are People" style crisp crack
/ Placed in real rooms, run through guitar amps — gives body not distortion
/ Synclavier + room sample combined
/ Character: very fast attack, moderate tonal body, clean
/ 250ms
N: 11025
T: !N

/ Body: two close tones (the "tuned" quality of Wilder's snare hits)
B: e(T*(0-40%N))
F: 210*(6.28318%44100)
G: 260*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
O: (s P)
I: (s Q)
S: B*((O+I)*.5)

/ Noise: clean, full duration, the amp/room character
E: e(T*(0-6.9%N))
R: r T
U: E*R

/ High-mid presence: the guitar amp EQ lift (~2-4kHz)
A: 0.5 f U
H: U-A

/ Very fast snap transient
V: e(T*(0-200%N))
K: r T
C: V*K

W: w (S*.6)+(U*.5)+(H*.3)+(C*.3)
