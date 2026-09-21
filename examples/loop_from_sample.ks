/ Loop Construction from a Loaded Sample
/ Demonstrates the X, L, and J verbs for finding and building loops

/ Assume W has been loaded via \load or \ra from a .wav file
/ For this demo, we synthesize a 4-second test tone with vibrato

N: 176400
T: !N
F: 220
Vib: s(T*(5*6.28318%44100)) * 3
W: s(T*((F+Vib)*6.28318%44100)) * 0.8

/ --- Step 1: Find zero crossings ---
/ X returns sample indices where the waveform crosses zero.
/ These are the safest places to start/end a loop (no click).
ZC: W x 0.001

/ --- Step 2: Score loop candidates ---
/ L scores how well each position matches itself one period later.
/ Here we test a 2-second loop (88200 samples) with a 512-sample window.
/ Lower score = better match = smoother loop.
SC: W l (88200 512)

/ --- Step 3: Find the best loop start ---
/ The minimum value in SC is the best loop start point.
/ In practice, pick a zero-crossing near that minimum.

/ --- Step 4: Crossfade the loop boundary ---
/ J blends the tail of the loop back into the head.
/ (start end xfade_samples) — 50ms crossfade at 44100Hz = 2205 samples
W2: W c (11025 99225 2205)

/ Now W2 has a seamless crossfade at the loop boundary.
/ Load it with:
\bm W2 33
\loop all 11025 99225
