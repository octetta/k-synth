/ 1. The Ramp and Envelope
T:!11025
E:(1-T*0.00009)^16

/ 2. The Kick: Frequency sweep (T*0.01) multiplied by a dropping factor
/ We use E to drop the pitch and the volume simultaneously
K:E*s(T*0.01)
