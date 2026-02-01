/ --- The Sub Kick ---
B:!22050
/ Pitch: Linear drop into sub territory
P:s B*0.04*(1-B%22050)
/ E: Exponential decay
E:(1-B%22050)^2
/ K: The Filter makes it heavy; Tanh makes it loud
K:h 2*f E*P

/ --- The Snappy Snare ---
R:r 11025
/ W: Filtered noise creates a 'hollow' body
W:f R*(1-B%11025)^4
/ C: Unfiltered noise crack
C:0.4*(r 11025)*(1-B%11025)^15
S:h W+C

/ --- Sequencer ---
T:!44100
J:_T%11025
O:((J!2)=0*44100#K)+((J!2)=1*44100#S)
\p O
