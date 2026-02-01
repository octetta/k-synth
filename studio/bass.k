/ bass.k
T: ! 44100
/ 100Hz Sine Wave
S: s T * 2 * p * 100 % 44100
/ Apply a smooth window to the entire second
W: w S
O: S * W
play O
