/ --- 1. The Sawtooth Oscillator ---
/ A: Frequency factor (tuning)
A:0.015
/ M: Generate a raw sawtooth wave
M:((!44100) * A) ! 1
/ Center it between -0.5 and 0.5
M:M - 0.5

/ --- 2. The Filter Envelope (The 'Squelch') ---
/ We want the filter to snap shut on every 1/8th note
/ B: A 1/8th note ramp (5512 samples)
B:(!5512) % 5512
/ E: A sharp decay curve
E: (1 - (B % 5512) % 5512) ^ 4
/ C: Scale the envelope to a good cutoff range (0.05 to 0.4)
C: 0.05 + (0.35 * E)

/ --- 3. The Resonance ---
/ R: Set this high (around 3.0) for that whistling acid sound
R:3.0

/ --- 4. The Final Synthesis ---
/ Combine Cutoff and Resonance into a matrix
/ Since C is an array and R is a scalar, dyadic f handles it
X:(C,R) f M

/ Drive it hard to make it growl
X:h 4 * X
\p X
