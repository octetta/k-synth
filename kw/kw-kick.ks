/ Kraftwerk Kick — Maestro Rhythm-King / Simmons SDS-V style
/ "The Man Machine", "Numbers", "Computer World"
/ Source: triangle VCO + LP-filtered noise + click transient
/ Triangle oscillator = clean, slightly warm, not the rounded sine of 808
/ SDS-V bend control = pitch sweep amount and speed
/
/ 350ms — Kraftwerk kicks are precise and don't over-sustain
N: 15435
T: !N

/ Triangle oscillator: approximate with $ (odd harmonics, 1/h^2, alternating sign)
/ Pitch sweep: 120Hz -> 50Hz, bend speed medium (Synthanorma precision)
F: 50+70*e(T*(0-50%N))
D: F*(6.28318%44100)
P: +\D
/ Triangle spectrum: h1=1, h3=-1/9, h5=1/25, h7=-1/49
A: 1 0 -.111 0 0.04 0 -.0204
S: P $ A

/ SDS-V "click drum" — extra attack from pad impact
/ Very fast noise burst through SSM2044 filter (LP)
V: e(T*(0-500%N))
R: r T
C: 0.3 f R
K: V*C

/ Noise component (SDS-V noise level control): LP-filtered white noise
/ Synthanorma used "high resonant frequencies" — slight resonance on LP
E: e(T*(0-6.9%N))
U: r T
/ Resonant LP at ~200Hz gives the Simmons "body thud" quality
L: 0.04 0.6 f U
X: E*L

/ Synthanorma clipping: d() on the triangle body
/ "clipped, high resonant frequencies to boost the rhythm"
O: d(S*1.5)

W: w (E*O*.9)+(X*.25)+(K*.3)
