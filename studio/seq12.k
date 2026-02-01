/ --- The 808 Kick (The 'Juice') ---
B:!22050
E:1-B%22050
K:h 3*(s B*0.01*E)*E

/ --- The 'Firestarter' Snare (Surgical) ---
/ 1. The Snap: Raw noise filtered to remove the 'whiteness'
/ We use 'f' twice to make the slope steeper (2-pole filter)
R:r 11025
W:f f 5*R*(1-!11025%11025)^4

/ 2. The Bite: A 1000Hz 'ping' that creates the crack
/ (Multiplier 0.15 is roughly 1kHz at 44.1k)
P:s (!11025)*0.15*(1-!11025%11025)^20

/ 3. The Body: A 200Hz 'thud' 
T:s (!11025)*0.03*(1-!11025%11025)^5

/ Mix: Smash the Bite and Snap together, then add the Thud
S:h 4*(0.8*W + 0.5*P) + 0.3*T

/ --- The Sequence ---
O:K,S,K,S
\p O
