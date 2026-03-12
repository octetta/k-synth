/ NAP noise sweep — rising filtered noise, tension builder
/ like a machine powering up badly
N: 44100
T: !N
R: r T
/ filter cutoff rises over duration
O: T*(1.0%N)
/ slow sweep from very dark to bright
F: (0.02+0.6*O) f R
E: e(T*(0-0.5%N))
W: w d(E*F*2.5)
