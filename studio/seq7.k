/ --- The Solid Thump (Improved Pitch) ---
B:!11025
E:(1-B%11025)^5
/ We keep your 0.08, but make the pitch drop even more 'rubbery'
K:E * s B * 0.08 * (0.1 + E^3)

/ --- The Snare (Layered Crack) ---
/ Layer 1: The sharp spike you had before
W:(r 11025) * (1-B%11025)^12
/ Layer 2: A slightly longer 'hiss' for body
H:(r 11025) * (1-B%11025)^5
/ Layer 3: Your 180Hz 'knock'
P:(s B * 0.25) * (1-B%11025)^30

/ Combine: 0.6 Spike + 0.4 Hiss + 0.3 Pop
S:h (0.6*W) + (0.4*H) + (0.3*P)

/ --- The Original Working Sequencer ---
T:!44100
J:_T%11025
M:(J!2)=0
N:(J!2)=1
O:(M*44100#K) + N*44100#S

\p O
\w 1000
\p O
