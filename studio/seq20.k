/ --- The 'Actual' Snare ---
/ 1. The Envelope: Force the decay with parens
/ This creates a ramp from 1 down to 0
E:1 - ((!11025)%11025)
/ 2. The Sharpness: Square it to make it a curve
E:E * E
/ 3. The Noise: Short burst
R:r 11025
/ 4. The Hit: Multiply them (This MUST be changing volume now)
S:R * E
/ 5. The Filter: Muffle it slightly
S:0.2 f S
\p S
