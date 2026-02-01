/ 1. Kick: 0.25s sine sweep with decay
B:!11025
E:1-B%11025
K:0.8*E*s B*0.05

/ 2. Snare: 0.25s noise pulse
S:0.5*E*r 11025

/ 3. Sequencer: 1 second at 44.1k
T:!44100
J:_T%11025
/ M is mask for beats 0,2; N is mask for beats 1,3
M:(J!2)=0
N:(J!2)=1
/ Reshape K and S to 44100 and apply masks
O:(M*44100#K)+N*44100#S

play O
