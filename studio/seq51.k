/ --- 1. Timing & Note ---
N:22050
L:!N
/ Osc: Centered Sawtooth (400Hz)
M:((L ! 400) * 0.01) - 0.5

/ --- 2. The Squelch (P) ---
/ Cutoff: Drops from high to low
C:0.05 + 0.4 * (1 - L/N) ^ 2
/ Resonance: Increases as the filter closes (Aggressive)
R:N # 3.85
/ Join into the dual-parameter array
P:C,R

/ --- 3. The Signal Path ---
X:P f M
/ Drive it into the tanh 'h'
X:h 6 * X

/ --- 4. The 'Pump' (Sidechain) ---
/ Duck the volume at the start of the beat
G:(L % 5512) / 5512
X:X * G * G

\p X
