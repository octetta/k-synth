\l setup.k

/ Pitch envelope: starts high, drops fast
T: 1 - 0.0008 * A
/ Exponential drop
T: T ^ 3

/ Sine wave that follows pitch envelope  
/ Starts ~120 Hz, drops to ~40 Hz
R:44100
X:120 % R
Y:40 % R
B: s p (X + (Y * T)) * A

/ Apply amplitude envelope
K: 0.8 & -0.8 | 5 * B * M * M

