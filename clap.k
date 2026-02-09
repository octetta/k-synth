\l setup.k

/ Multiple noise bursts for "flam"
N: 0.55 f r A

/ Modulation for texture
L: s 45 * A

/ Create burst pattern
C: N * (P + (0.25 * P * L) + (0.15 * P * P))

