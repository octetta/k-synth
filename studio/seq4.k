/ --- The 'Weighty' Kick ---
B:!22050
/ Pitch: Rapid drop from high to low
P:s B * 0.05 * (1-B%22050)^2
/ Envelope: Very long tail
E:(1-B%22050)^2
/ Filter: Apply the 'f' verb to smooth the sine into a sub-bass thump
K:f E*P
/ Saturate
K:h 1.5*K

/ --- The 'Wood-Block' Snare ---
R:r 11025
/ Filter the noise! This turns 'static' into a 'hollow' drum sound
W:f R*(1-B%11025)^4
/ Add a high-pitched 'crack'
C:0.5*(r 11025)*(1-B%11025)^15
S:h W+C

/ --- The Sequencer ---
T:!44100
J:_T%11025
O:((J!2)=0*44100#K)+((J!2)=1*44100#S)
\p O
