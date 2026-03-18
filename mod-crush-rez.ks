/ MODULATED CRUSH & RESONANCE
N:22050
T:!N

/ Pitch Envelope
P:e T*0-20%N

/ Level Envelope (L): 
/ Starts at 2 levels (brutal) and climbs to 64 levels (clean)
/ 1-e... goes 0 -> 1, so we scale it.
L:2+62*1-e T*0-30%N

/ Amplitude Envelope
E:e T*0-8%N

/ Core Kick
K:(s T*(50+350*P)*(p 2)%44100)*E

/ Dynamic Bit Crush: Array 'L' defines levels per sample
C:L v K

/ Short Delay for "Metal" Resonance (800 samples ~ 18ms)
D:(800 0.45) y C

/ Output with Saturation
W:w h D
