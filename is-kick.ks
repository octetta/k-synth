/ InSoc Kick — E-MU SP-12 "Bass Drum" preset
/ "What's On Your Mind", "Walking Away", "Repetition"
/ SP-12 kick: sampled acoustic kick at 12-bit/26kHz
/ Static 5-pole Chebyshev filter on bass channel: low-pass at ~150Hz
/ The "big boom" sound = kick layered with tuned-down SP-12 tom
/
/ 12-bit character: quantize body after synthesis
/ 26kHz imaging: content above 13kHz folds back → brightness/grit
/
/ 350ms — SP-12 kicks had punchy medium sustain
N: 15435
T: !N
E: e(T*(0-6.9%N))

/ Acoustic kick body: fast pitch sweep (sampled real kick)
/ 26kHz Nyquist = original kick had content around 100-150Hz
F: 55+85*e(T*(0-80%N))
D: F*(6.28318%44100)
P: +\D
S: s P

/ SP-12 static Chebyshev LP on bass channel — fixed cutoff ~150Hz
/ Very tight LP that gives the "boom" but cuts high-mid
C: 0.025 f S

/ 12-bit quantization grit — the SP-12 character on the sine body
/ 4096 levels (12-bit): subtle but audible on the low-frequency body
Q: 4096 v C

/ The SP-12 kick also had a noise "crack" component in the sample
/ Short transient from the beater impact (part of the original sample)
V: e(T*(0-400%N))
R: r T
/ Static filter on crack component
L: 0.2 f R
K: V*L

/ Sub layer: "way tuned down SP-12 tom" — the InSoc secret sauce
/ Tom channel SSM2044 dynamic filter opening on attack
/ Tom at ~60Hz (tuned way down from nominal 200Hz tom)
G: 60*(6.28318%44100)
M: +\(N#G)
/ Dynamic filter: bright attack (ct=0.3), decays to dark (ct=0.04)
/ Approximate: blend of two filtered versions
I: 0.3 f (s M)
J: 0.04 f (s M)
B: e(T*(0-40%N))
X: (B*I)+((1-B)*J)

W: w (E*Q*.85)+(K*.25)+(E*X*.4)
