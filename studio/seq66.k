N:22050
L:!N
M:((L ! 400) * 0.01) - 0.5
C:N # 0.15
/ X is Low Resonance
R:N # 0.5
X:(C,R) f M
/ Y is High Resonance
S:N # 3.7
Y:(C,S) f M
/ Concatenate X and Y into one 44,100 sample array
Z:X,Y
\p Z
