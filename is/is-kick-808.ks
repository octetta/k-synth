/ InSoc Kick — TR-808 style (used on "Running")
/ "Running" used a TR-808 kick married to a Moog Prodigy bass synth
/ Classic 808 sub kick with extra low-end reinforcement
/ The Prodigy bass tone adds a sub-octave "swelling" quality
/ 400ms
N: 17640
T: !N
E: e(T*(0-6.9%N))

/ 808-style sine kick: long sweep, deep sub
F: 50+100*e(T*(0-50%N))
D: F*(6.28318%44100)
P: +\D
S: s P

/ 12-bit quantize for SP-12/808 era character
Q: 4096 v S

/ Moog Prodigy "bass synth married" component:
/ The Prodigy had a raw sawtooth or square tone
/ We use a very low near-triangle tone at sub frequency
/ Slow attack following the kick (the bass swelling in)
G: 42*(6.28318%44100)
M: +\(N#G)
A: 1 0 -.111 0 0.04
B: (M $ A)*e(T*(0-4%N))

/ Click: the 808 beater transient
V: e(T*(0-500%N))
R: r T
C: 0.1 f R
K: V*C

W: w (E*Q*.9)+(B*.3)+(K*.2)
