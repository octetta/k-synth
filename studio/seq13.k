/ --- The 808 Kick (Keep the Juice) ---
B:!22050
E:1-B%22050
K:h 3*(s B*0.01*E)*E

/ --- The 'Sledgehammer' Snare ---
/ 1. The Snap (Impulse): 0.1s of noise with a BRUTAL decay
R:r 4410
/ ^20 makes the noise hit like a hammer and vanish
W:R*(1-!4410%4410)^20

/ 2. The Ring (Body): 0.1s of a 300Hz tone
/ We use a 'distorted' sine to give it that 'wood' sound
T:h 2*s(!4410)*0.04*(1-!4410%4410)^8

/ 3. The Smash: Combine them at high volume
S:h 5*(W+T)

/ --- The Master Sequence ---
/ We need a 'blank' space (Z) to make the hits distinct
Z:0*!11025
/ Sequence: Kick, Silence, Snare, Silence
O:K,Z,S,Z
\p O
