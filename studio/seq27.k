/ --- Setup: Quick Kit Refresh ---
B:!11025
K:(h 3 * (s B * 0.01) * (1-(B%11025)))
M:(s B * 0.008) * (1-(a K))
S:(h 4 * (0.3 f (r 11025)) * (1-(B%11025))^12)
/ Hat
H:( (r 5512) - (0.1 f (r 5512)) ) * (1-((!5512)%5512))^15
P:H,H

/ --- Loop Z: The Main Beat (4/4 Drive) ---
Z:(K+M),P,(S+M),P,K,(K+M),S,P

/ --- Loop Y: The Breakdown (Syncopated) ---
/ We swap some kicks for silence (0*B) to let the bass breathe
Y:(K+M),P,S,P,(0*B),P,(S+M),P

/ --- The Master Track (X) ---
/ Play the main beat twice, then the breakdown, then the main beat again
X:Z,Z,Y,Z

/ --- Final Master Polish ---
/ Apply a slight room echo (600 sample delay) and master gain
X:600 y (h 1.2 * X)

\p X
