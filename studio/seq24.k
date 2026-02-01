/ --- The 'Duck' Factor ---
/ K is our kick. We want the bass to be quiet when K is loud.
/ D will be 1 where there is no kick and 0 where the kick is loudest.
D:1 - (a K)

/ --- The Bass Tone (M) ---
/ A low sine wave (55Hz = A1)
/ 55Hz at 44.1k is roughly 0.008 radians per sample
B:!11025
M:s B * 0.008
/ Apply the Ducking: Bass * DuckFactor
M:M * D

/ --- The 'Prodigy' Mix ---
/ We'll make the bass louder and saturate it
M:h 3 * M

/ --- The Combined Sequence ---
/ We add the bass (M) to every slot that has a kick (K) or ghost (G)
/ Using , to join the slots
O:(K+M),G,(S+M),H,(K+M),K,(S+M),I
\p O
