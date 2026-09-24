/ 80s Seamless Looping Choir Pad
/ Uses phase-locked detuning to guarantee a clickless infinite loop

F: 55

/ We want our loop to be exactly 2 seconds long (88200 samples)
L_loop: 88200
/ 0.25s attack envelope
L_att: 11025
/ 1s release tail envelope
L_rel: 44100

N: L_loop + L_att + L_rel
T: !N

/ --- Phase-Locked Detune Verb ---
beat_detune: { y % ((x%44100)*F) }
detune: L_loop beat_detune 1

F1: F * (1 - detune)
F2: F * (1 + detune)

P1: T*(F1*6.28318%44100)
P2: T*(F2*6.28318%44100)

/ Generate the formants
H: !64
H: H+1
D: H-13
B: H-20
C: H-45
A: (1%H) * ((e(0-D*D*.1)) + ((e(0-B*B*.1))*.8) + ((e(0-C*C*.05))*.5))

W1: w P1 $ A
W2: w P2 $ A
W_mix: W1 + W2

/ Attack and Release Envelope
E_att: T%11025
E_rel: 1 - ((T-(L_loop+L_att))%L_rel)
Env: ((T<11025)*E_att) + (((T>11024)*(T<(L_loop+L_att)))*1.0) + ((T>(L_loop+L_att-1))*E_rel)

W: W_mix * Env * 0.2

\bm W 33
\loop all 11025 99225
