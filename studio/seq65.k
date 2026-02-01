N:22050
L:!N
/ M: Sawtooth Oscillator
M:((L ! 400) * 0.01) - 0.5
/ C: Constant Cutoff
C:N # 0.15
/ X: Low Resonance Segment (Res = 0.5)
R:N # 0.5
X:(C,R) f M
/ Y: High Resonance Segment (Res = 3.7)
S:N # 3.7
Y:(C,S) f M
/ Join and Play: 2 seconds of audio
\p X,Y
