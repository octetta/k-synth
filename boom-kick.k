\l setup.k

/ Ultra-low 35 Hz
B: s p 0.00159 * A

/ Slow envelope with gentle curve
S: 1 - 0.00005 * A

/ Soft saturation
K: h 4 * B * S * S
