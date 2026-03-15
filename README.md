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

Sound is a vector. A kick drum is a vector. A two-second bell tone is a
vector. You do math on vectors and the result is audio. There are no tracks,
no timelines, no patch cables — only expressions.

---

### 🚀 Quick Start

To hear the engine in action, try these incantations:

```
/ make a simple kick drum
A: !8000
E: 1 - 0.000125 * A
K: 0.5 & -0.5 | 5 * (s 0.007 * A) * E * E

/ now a snare drum
N: 0.5 f r A
P: E * E * E * E * E
T: (s 0.2 * P * A) * P
S: 0.7 & -0.7 | (2 * T) + N * E * E

/ now closed and open hi-hats
C: (0.95 f r A) * P
O: (0.95 f r A) * (0.8 * E) + (0.5 * P)

/ now make a pattern
W: K,K,S,K,K,S,K,K,S,C,C,C,O,S,S
```

---

### 🔧 Build

```sh
gcc -O2 ksynth.c -lm -o ksynth
```

The engine is a single C file with no dependencies beyond libc and libm.
It can also be embedded as a library: include `ksynth.h` and call `ks_run()`
per script line from any C program. It compiles to WebAssembly via Emscripten
for browser use.

---

### 🎛️ What It Can Do

#### Oscillators

Any waveform you can express mathematically. The phase accumulator pattern
`+\(N#F)` where `F` is a per-sample phase increment gives a clean oscillator
at any frequency. Apply `s` for sine, `c` for cosine, or do math on the raw
ramp for triangle and sawtooth shapes.

```
/ sawtooth at 220 Hz, 1 second
N: 44100
T: !N
F: 220%44100
P: +\(N#F)
W: w (P*2)-1
```

#### FM Synthesis

Right-associativity makes FM natural. `s P + Q` parses as `s(P + s(Q))` —
that is the FM formula. Carrier phase plus modulator sine, with no extra
syntax required.

```
/ FM bell: fast index decay, slow amplitude decay
N: 88200
T: !N
A: e(T*(0-3%N))
I: 3.5*e(T*(0-40%N))
C: 440*(6.28318%44100)
M: 440*(6.28318%44100)
P: +\(N#C)
Q: +\(N#M)
W: w A*(s P+(I*s Q))
```

Vary the carrier-to-modulator ratio for different timbres: `1.0` is warm and
round, `1.4` is metallic and bell-like, `3.5` is tubular and bright.

#### Additive Synthesis

The `o` and `$` operators sum harmonic series directly. `P o H` sums
`sin(P×h)` for each harmonic h in H at equal amplitude. `P $ A` weights each
harmonic by a corresponding amplitude in A.

```
/ organ tone: odd harmonics at equal amplitude
N: 44100
T: !N
F: 220*(6.28318%44100)
P: +\(N#F)
H: 1 3 5 7
W: w P o H

/ cello-ish: weighted harmonic series
A: 1 0.6 0.4 0.25 0.15 0.08
W: w P $ A
```

#### Envelopes and Dynamics

`e(T*(0-k%N))` gives a pure exponential decay from 1 to `e^-k` over N
samples. Adjust `k` to control the decay time. For a percussive rise-and-fall
shape, `T*e(T*(0-k%N))` peaks at sample `N/k` then decays naturally.

Soft clipping with `d` (`tanh(3x)`) adds saturation and tames peaks without
hard discontinuities. Hard clipping uses min/max: `x & -limit | limit`.

#### Filters

Two-pole lowpass via `ct f signal` where `ct` is a normalized cutoff (0–1).
Subtract a lowpass from the original signal for highpass. Subtract two
lowpass filters for bandpass.

```
/ bandpass filtered noise
N: 44100
T: !N
R: r T
H: 0.7 f R      / lowpass ~7 kHz
L: 0.08 f R     / lowpass ~600 Hz
B: H-L          / band between them
W: w B
```

#### Noise and Percussion

`r T` generates white noise (N samples when T is a length-N index vector).
`m T` generates 1-bit noise — alternating `±0.7` — with a harder, metallic
timbre useful for cymbals and hi-hats.

Combine noise, filtered noise, and sine sweeps to build any percussion voice:

```
/ kick: pitch-swept sine + noise transient
N: 13230
T: !N
F: 50+91*e(T*(0-60%N))
D: F*(6.28318%44100)
P: +\D
S: (s P)*e(T*(0-6.9%N))
R: 0.5 f r T
E: e(T*(0-40%N))
W: w (S+(R*E))
```

#### Patterns and Sequencing

The `,` operator concatenates vectors. A bar of drums is the concatenation
of individual voice vectors in sequence:

```
Z: K,K,S,K,K,S,K,K,S,C,C,C,O,S,S
```

Longer patterns, fills, and arrangement are vector math — tile with `#`,
mix with weighted addition, chain bars with more `,`.

#### Feedback Delay and Comb Filtering

`[d g] y signal` applies a feedback delay of `d` samples with gain `g`.
Values of `g` near 1.0 give long, resonant comb filtering. Lower values
give short slap echoes. Use it to add metallic resonance to noise or space
to any signal.

#### Convolution

`A z B` convolves two vectors. Design FIR filters, apply impulse responses,
or build custom reverb tails from measured room samples.

---

### 🌐 Web Interface

k/synth compiles to WebAssembly via Emscripten and runs in the browser as a
live-coding notebook. The web interface provides:

- A **notebook** that logs every script run with waveform display and one-click replay
- **16 sample slots** in a 2×8 grid to hold synthesized buffers
- A **pad panel** — a 4×4 trigger grid with per-pad pitch control
- **Melodic mode** — all 16 pads play one slot at different semitone offsets, turning any synthesized patch into a playable instrument
- A **patches browser** that fetches `.ks` files directly from this repo

```sh
source /path/to/emsdk/emsdk_env.sh
bash build.sh
python3 -m http.server 8080
```

---

### 📖 Language Quick Reference

| Concept | Example | Notes |
|---------|---------|-------|
| Index vector | `!N` | `[0, 1, …, N-1]` |
| Phase accumulator | `+\(N#F)` | Oscillator at frequency F |
| Sine / cosine | `s P` / `c P` | Elementwise |
| Exponential decay | `e(T*(0-k%N))` | Decay from 1 to e^-k over N |
| White noise | `r T` | One sample per element of T |
| 1-bit noise | `m T` | `±0.7`, metallic timbre |
| Lowpass filter | `ct f sig` | Two-pole, ct=0..1 |
| Additive equal | `P o H` | Sum harmonics in H |
| Additive weighted | `P $ A` | Weighted harmonic series |
| FM synthesis | `s P+(I*s Q)` | Right-assoc gives FM naturally |
| Concatenate | `A,B,C` | Build patterns |
| Tile | `N#V` | Repeat V to length N |
| Soft clip | `d x` | `tanh(3x)` |
| Normalize | `w x` | Scale to peak ±1 |
| Feedback delay | `[d g] y sig` | Comb / echo |
| Convolution | `A z B` | FIR / impulse response |

Right-associativity: `a op b op c` = `a op (b op c)`. Use parentheses for
linear mixes: `(A*0.5)+(B*0.5)`, not `A*0.5+B*0.5`.

---

### ❤️ Credits

🧮 [ksimple](https://github.com/kparc/ksimple) *tiny k by arthur whitney*

🔊 [miniaudio](https://github.com/mackron/miniaudio) *audio library by mackron*

🖥️ [bestline](https://github.com/jart/bestline) *command session by jart*

**Built** by Joseph Stewart [octetta](https://octetta.com) 🐙♊ in collaboration with AI-🧠
pair programming.
