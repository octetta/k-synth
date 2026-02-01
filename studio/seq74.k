N:44100
L:!N
/ Oscillator: Sawtooth is best for vocals (rich in harmonics)
M:((L ! 400) * 0.01) - 0.5

/ C: First Formant (F1) - Stays relatively low
C:0.1 + 0.05 * s L % 8000
/ D: Second Formant (F2) - Sweeps much higher
D:0.3 + 0.2 * s L % 8000

/ R: High Resonance (Essential for formants)
R:N # 3.8

/ Apply both filters and sum them
/ This creates two resonant peaks in the spectrum
X:(C,R) f M
Y:(D,R) f M

/ Z: The Vocal Result (normalized)
Z:0.5 * X + Y
\p Z
