/ --- The Kit ---
B:!11025
K:(h 3 * (s B * 0.01) * (1 - (B % 11025)))
M:(s B * 0.008) * (1 - (a K))
S:(h 4 * (0.3 f (r 11025)) * (1 - (B % 11025))^12)
/ Hat (double-speed)
N:r 5512
H:(N - (0.1 f N)) * (1 - ((!5512)%5512))^15
P:H,H

/ --- The Master Loop (Z) ---
/ We assemble the pattern into one single variable
Z:(K+M),P,(S+M),P,K,(K+M),S,P

/ --- The Remix (The 'Effects Bus') ---
/ 1. Add some global 'air' with a very short slapback
Z:200 y Z
/ 2. Slam the whole thing through a final Tanh for 'mastering'
Z:h 1.5 * Z

\p Z
