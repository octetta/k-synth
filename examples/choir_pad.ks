/ 80s Pop Choir Pad
/ Uses 64 harmonics and additive synthesis to model an "AAH" vocal formant
/ Baked-in 2-second ADSR envelope and dual-oscillator detuned chorusing

/ 2 seconds of audio at 44100Hz
N: 88200
T: !N

/ Base Frequency: 55Hz (A1)
/ A low base frequency gives us a dense harmonic spectrum to sculpt
F: 55

/ Constructing the Formant Spectrum
/ AAH formants are typically around 700Hz, 1100Hz, and 2500Hz
/ At 55Hz, these align with harmonics 13, 20, and 45.
H: !64
H: H+1
D: H-13
B: H-20
C: H-45

/ Gaussian curves to create peaks at the formant frequencies
/ NOTE: Monadic operators in KSynth (like 'e') are right-associative and bind to the ENTIRE rest of the expression!
/ Parentheses are required around (e(...)) before multiplying to scale amplitude!
A: (1%H) * ( (e(0-D*D*.1)) + ((e(0-B*B*.1))*.8) + ((e(0-C*C*.05))*.5) )

/ Dual Detuned Oscillators (+/- 0.8%) for that classic 80s thick chorus
P1: T*(F*0.992*6.28318%44100)
P2: T*(F*1.008*6.28318%44100)
W1: w P1 $ A
W2: w P2 $ A
W_mix: W1 + W2

/ Baked-in ADSR Envelope
/ Attack (0.25s), Decay (0.25s), Sustain (0.8), Release (1.0s)
E_att: T%11025
E_dec: 1 - ((T-11025)%11025)*0.2
E_sus: 0.8
E_rel: 0.8 - ((T-44100)%44100)*0.8

/ Piecewise assembly of the envelope
Env: (T<11025)*E_att + (T>=11025)*(T<22050)*E_dec + (T>=22050)*(T<44100)*E_sus + (T>=44100)*E_rel

/ Final Output Wave (scaled down to prevent clipping)
W: W_mix * Env * 0.2

/ To play with MIDI across the keyboard, map it to the base note A1 (MIDI 33):
/ \bm W 33
