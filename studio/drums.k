/ --- Kick (Solid Thump) ---
B:!11025
E:(1-B%11025)^5
K:E * s B * 0.08 * (0.1 + E^2)

/ --- The Snare (High-Contrast Crack) ---
/ 1. The 'Crack': Pure, unadulterated noise, very short
/ We use a massive exponent (^12) so it 'spikes' then dies
W:(r 11025) * (1-B%11025)^12

/ 2. The 'Pop': A 180Hz sine 'knock'
/ We use ^30 so it's only a few milliseconds long
/ This provides the 'thwack' of the stick
P:(s B * 0.25) * (1-B%11025)^30

/ 3. Combine: Noise is the leader here (0.8 vs 0.4)
S:(0.8*W) + (0.4*P)

/ --- Sequencer ---
T:!44100
J:_T%11025
M:(J!2)=0
N:(J!2)=1
O:(M*44100#K) + N*44100#S

\p O
\w 1000
\p O
