/ --- Setup for the Layer ---
T:44096
I:!T
/ M: Sawtooth Source for the synth
W:((I ! 400) * 0.005) - 0.25

/ C: Modulated Cutoff for that liquid feel
C:0.15 + 0.1 * s I % 10000
R:T # 3.5

/ B: The Bassline (Filtered Sawtooth)
B:(C,R) f W

/ --- Master Mix ---
/ We add the Drums (Z) and the Bassline (B)
/ Scale by 0.5 to prevent clipping
Q:0.5 * Z + B
\p Q
