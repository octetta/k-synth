/ studio.k
/ 1. Create a single 1/4 second 'beat' buffer
B: ! 11025
W: w B

/ 2. THE KICK (Low Boom)
/ s B * 0.04 creates a ~280Hz tone; W seals the edges
K: W * s B * 0.04

/ 3. THE SNARE (Crispy Crack)
/ 0.6 h r... high-passes white noise
S: W * 0.6 h r 11025 # 1

/ 4. SEQUENCE: 4 beats (Kick, Snare, Kick, Snare)
/ We reshape the instruments into 1-second loops
OK: 44100 # K
OS: 44100 # S

/ 5. MASKING: Silent periods so they alternate
MK: 44100 # (11025 # 1) + 11025 # 0
MS: 44100 # (11025 # 0) + 11025 # 1

/ 6. FINAL MIX
O: (0.5 * OK * MK) + 0.3 * OS * MS
play O
