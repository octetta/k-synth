N:44100
L:!N
/ Oscillator: Sawtooth
M:((L ! 400) * 0.01) - 0.5
/ Filter: High Resonance
C:N # 0.15
R:N # 3.7
F:(C,R) f M
/ Gate: 1 if (L % 4000) is less than 2000, else 0
/ We use ( (L ! 4000) % 2000 ) _ to floor it to a pulse
G:1 - (L ! 4000) % 2001 ^ 0
/ Apply gate to filtered signal
Z:G * F
\p Z
