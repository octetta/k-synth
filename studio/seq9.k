/ --- The 808 'Sub-Slam' ---
B:!22050
/ Pitch: Rapid drop from 120Hz to 40Hz
P:s B*(0.06*(1-B%22050)^2+0.008)
/ Env: Linear decay stays louder longer than exponential
E:1-B%22050
/ The Juice: Saturate heavily
K:h 5*P*E
/ Double it up for sub-harmonics
K:K+0.5*h 10*P*E^4

/ --- The 'Firestarter' Snare ---
/ R: Raw Noise
R:r 11025
/ The 'Crack': Filtered noise with high gain
/ This gives it that 'industrial' thickness
W:f 8*R*(1-!11025%11025)^3
/ The 'Ring': 400Hz tone for the drum shell
T:s (!11025)*0.3*(1-!11025%11025)^10
/ Mix and Crush
S:h 2*W+T

/ --- Sequencer ---
T:!44100
J:_T%11025
O:((J!2)=0*44100#K)+((J!2)=1*44100#S)
/ Final Master Gain
O:h 1.2*O

\p O
