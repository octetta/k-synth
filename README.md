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
To hear the engine in action, try these incantations:

```
/ make a sine wave 1 second @ 440Hz
W:s (!44100) * p2 * 440 % 44100
/ play it
\p W

/ now a kick drum
A: !8000
E: 1 - 0.000125 * A
K: 0.5 & -0.5 | 5 * (s 0.007 * A) * E * E
\p K

/ now a snare drum
N: 0.5 f r A
P: E * E * E * E * E
T: (s 0.2 * P * A) * P
S: 0.7 & -0.7 | (2 * T) + N * E * E
\p S

/ now closed and open hi-hats
C: (0.95 f r A) * P
O: (0.95 f r A) * (0.8 * E) + (0.5 * P)

/ now make a pattern
Z: K,K,S,K,K,S,K,K,S,C,C,C,O,S,S

/ save it to a wav file
\s Z
```

**hear Z**
https://github.com/user-attachments/assets/49376dbd-db15-4292-b93a-3fb42f8d78d6

**Built** by Joseph Stewart [octetta](https://octetta.com) 🐙♊ in collaboration with AI-🧠
pair programming.

### ❤️ Credits

🧮 [ksimple](https://github.com/kparc/ksimple) *tiny k by arthur whitney*

🔊 [miniaudio](https://github.com/mackron/miniaudio) *audio library by mackron*

🖥️ [bestline](https://github.com/jart/bestline) *command session by jart*
