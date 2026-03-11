/ 808-style Clap
/ 3 staggered noise bursts, bandpassed 800-1800Hz
/ 200ms at 44100 = 8820 samples

N: 8820
T: !N

/ bandpass each noise source: HP at 800Hz, LP at 1800Hz
/ burst A: early transient ~7ms (k=30), quiet
R: r T
A: R-(0.114 f R)
B: 0.256 f A
X: w (T*e(T*(0-30%N)))

/ burst B: mid burst ~13ms (k=15), medium
J: r T
C: J-(0.114 f J)
D: 0.256 f C
Y: w (T*e(T*(0-15%N)))

/ burst C: main clap ~33ms (k=6), dominant
K: r T
E: K-(0.114 f K)
F: 0.256 f E
Z: w (T*e(T*(0-6%N)))

W: w (X*B*.3)+(Y*D*.5)+Z*F
