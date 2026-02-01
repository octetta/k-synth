/ --- 1. The Acid + Delay (From our last step) ---
/ (Assuming A, B, D, X are defined as before)
/ Let's ensure X is exactly 4 notes long (22050 samples)
X:A,B,D,B
X:4134 y X

/ --- 2. The Sidechain Envelope (G) ---
/ We want the volume to be 0 at the start of the beat and 1 at the end.
/ Since our notes are 5512 samples, we'll duck every note.
L:!(X#1)
/ G: Goes from 0.0 up to 1.0 every 5512 samples
G:(L % 5512) / 5512
/ Make the ducking "stiffer" by squaring it
G:G * G

/ --- 3. Apply the Ducking ---
X:X * G

/ --- 4. Add the Kick Drum (K) back in ---
/ Let's make a simple kick to fill the holes we just made
K:h 3 * (s (!5512) * 0.01) * (1 - ((!5512)/5512))
/ Reshape K to match the length of our track X
K:(X#1) # K

/ --- 5. The Final Mix ---
/ Mix the pumping bass with the solid kick
X:X + K
X:h 2 * X
\p X
