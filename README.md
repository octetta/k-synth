# k/synth
<image src="ksynth.png" width="200">

> "A pocket-calculator version of a synthesizer."

**k/synth** is a minimalist, array-oriented synthesis environment. Heavily
inspired by the **K/Simple** lineage and the work of Arthur Whitney, it
treats sound not as a stream, but as a holistic mathematical vector.

### 📐 The Philosophy
This isn't a DAW; it's a vector-processing engine designed for "Base Camp"
signal processing.

It uses:
* **One-Letter Variables**: A-Z globals only.
* **Right-Associativity**: Expressions evaluate from right to left.
* **Vectorized Verbs**: Math applied to entire buffers at once.

### 🚀 Quick Start
To hear the engine in action, pass a single variable to the playback command:

```
N:44100   / number samples 1 second @ 44100Hz
L:!N      / ramp 0 .. 44099
F:440     / frequency
T:2 * p 0 / 2*Pi = Tau

/ make a sine wave 1 second @ 440Hz
S:s L * T * F % N

/ play it
\p S

/ now a saw
W: (L ! N % F) * 1 % N % F
\p W

/ now a square
Q: ((_ 2 * (L!100.22)*0.00997)*2)-1
\p Q
```

Built by Joseph Stewart aka **octetta** in collaboration with AI-assisted
pair programming.
