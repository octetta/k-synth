/ --- 1. Timing ---
N:44100
L:!N

/ --- 2. The Oscillator (LOW GAIN) ---
/ We use a very small amplitude (0.1) to keep the filter stable
M:((L ! 400) * 0.0005) - 0.1

/ --- 3. Simple Static Filter ---
/ No envelope, just a fixed cutoff (0.1) and resonance (2.0)
/ This is the "Safe Mode" for your ksynth engine
C:0.1
R:2.0
X:(C,R) f M

/ --- 4. Direct Output ---
/ No tanh, no multipliers. Just play the raw result.
\p X
