# k/synth

> "A pocket-calculator version of a synthesizer."

k/synth is a minimalist, array-oriented synthesis environment. Heavily inspired by the K/Simple lineage and the work of Arthur Whitney, it treats sound not as a stream, but as a holistic mathematical vector.

Sound is a vector. A kick drum is a vector. A two-second bell tone is a vector. You do math on vectors and the result is audio. There are no tracks, no timelines, no patch cables — only expressions.

---

## the language

**One-letter variables** — `A`–`Z` globals only.

**Right-associativity** — expressions evaluate right to left.

**Vectorised verbs** — math applied to entire buffers at once.

**`W` is the output** — every script must set `W: w ...` to produce audio.

**`p` constants** — `p0` = 44100 (sample rate). `pN` = N×π for N≥1. So `p1` = π, `p2` = 2π, `p3` = 3π, etc. Since `p` is a verb it applies element-wise: `p 2` = 2π, `p !4` = `[44100, π, 2π, 3π]`.

The idiom `p2%p0` = 2π/44100 — the per-sample radian increment for 1 Hz — is the cleanest way to express the phase accumulator constant:

```
C: p2%p0               / 2π / 44100
P: +\(N#(440*C))       / phase ramp for 440 Hz over N samples
```

**User-defined functions** — `{ expression }` defines a function. Inside, `x` is the first argument and `y` is the second. Call with `arg1 FuncVar arg2` for two arguments or `FuncVar arg` for one.

The phase accumulator as a reusable function:

```
C: p2%p0
X: { +\(x#(y*C)) }    / x = n_samples, y = freq_hz
P: N X 440             / phase ramp for 440 Hz
Q: N X 445             / phase ramp for 445 Hz — 5 Hz beating with P
R: N X 227             / slightly detuned octave below
```

Functions eliminate repetition in multi-voice scripts and make the intent readable. Define the function once, call it for each voice.

---

## quick start

```
/ sine wave, 1 second, 440 Hz
N: 44100
T: !N
W: w s +\(N#(440*(p2%p0)))
```

Press `Ctrl+Enter` to run. A cell appears in the notebook with a waveform. Click `→0` to bank to slot 0. Click the slot to play it.

---

## what it can do

### wavetable oscillator

`table t freq dur` — plays `table` as a DDS oscillator at `freq` Hz for `dur` samples, with linear interpolation. `freq` and `dur` form a two-element vector — scalar variables following a number are absorbed into the vector literal, so `T t 440 D` works naturally.

Build tables using the phase accumulator pattern, using a separate variable for table size vs output duration:

```
/ sine at 440 Hz for 2 seconds
N: 1024
P: +\(N#(p2%N))
T: s P
D: 88200
W: w T t 440 D
```

```
/ sawtooth at 220 Hz for 1 second
N: 1024
P: +\(N#(1%N))
T: (2*P)-1
D: 44100
W: w T t 220 D
```

```
/ triangle at 330 Hz for 1 second
N: 1024
P: +\(N#(1%N))
T: (2*a((2*P)-1))-1
D: 44100
W: w T t 330 D
```

```
/ square wave at 220 Hz for 1 second
N: 1024
P: +\(N#(1%N))
T: (2*(P<0.5))-1
D: 44100
W: w T t 220 D
```

```
/ FM wavetable at MIDI note 69 (A4) for 2 seconds
N: 1024
P: +\(N#(p2%N))
I: 2.5
T: s P+(I*s P)
M: n69          / MIDI note 69 = 440 Hz; assign to M so "T t M D" works
D: 88200
W: w T t M D
```

Monadic `t` remains `tan`.

### oscillators

The phase accumulator pattern `+\(N#F)` where `F` is a per-sample phase increment gives a clean oscillator at any frequency. Apply `s` for sine, `c` for cosine, or do math on the raw ramp for triangle and sawtooth.

```
/ sawtooth at 220 Hz, 1 second (via harmonic sum)
N: 44100
T: !N
F: 220*(p2%p0)
P: +\(N#F)
H: 1 0.5 0.333 0.25 0.2 0.167
W: w P $ H
```

### FM synthesis

Right-associativity makes FM natural. `s P + Q` parses as `s(P + s(Q))` — carrier phase plus modulator sine.

```
/ FM bell: fast index decay, slow amplitude decay
N: 88200
T: !N
A: e(T*(0-3%N))
I: 3.5*e(T*(0-40%N))
C: 440*(p2%p0)
M: 440*(p2%p0)
P: +\(N#C)
Q: +\(N#M)
W: w A*(s P+(I*s Q))
```

Vary the carrier-to-modulator ratio: `1.0` is warm and round, `1.4` is metallic, `3.5` is tubular.

### additive synthesis

`P o H` sums `sin(P×h)` for each harmonic `h` in `H` at equal amplitude. `P $ A` weights each harmonic by a corresponding amplitude in `A`.

```
/ organ: odd harmonics
H: 1 3 5 7
W: w P o H

/ cello-ish: weighted series
A: 1 0.6 0.4 0.25 0.15 0.08
W: w P $ A
```

### envelopes

`e(T*(0-k%N))` gives exponential decay from 1 over N samples. `T*e(T*(0-k%N))` gives a percussive rise-and-fall shape peaking at sample `N/k`.

**Exponential decay** — a sine tone that fades out over 2 seconds:

```
N: 88200
T: !N
A: e(T*(0-3%N))
P: +\(N#(440*(p2%p0)))
W: w A*s P
```

Adjust the `3` to taste — larger decays faster, smaller lingers longer. At `k=1` the decay is very slow; at `k=10` it's a short pluck.

**Percussive rise-and-fall** — a thump that swells briefly then fades:

```
N: 44100
T: !N
A: T*e(T*(0-8%N))
P: +\(N#(180*(p2%p0)))
W: w A*s P
```

The peak lands at sample `N/k` — here `44100/8` ≈ 5500 samples ≈ 125ms in. Good for kick and tom shapes.

**Two envelopes on one voice** — fast index decay for a bright attack, slow amplitude decay for the body (the FM bell pattern):

```
N: 88200
T: !N
A: e(T*(0-3%N))
I: 3.5*e(T*(0-40%N))
C: 440*(p2%p0)
M: 440*(p2%p0)
P: +\(N#C)
Q: +\(N#M)
W: w A*(s P+(I*s Q))
```

`A` decays slowly (the ring). `I` decays fast (the clang). The combination is what makes it sound like a struck bell rather than a plain FM tone.

**Soft clipping** — `d` applies `tanh(3x)`, rounding peaks without hard discontinuities. Useful after loud envelopes:

```
N: 44100
T: !N
A: T*e(T*(0-5%N))
P: +\(N#(220*(p2%p0)))
W: w d A*s P
```

### filters

Two Chamberlin-derived lowpass filters — same topology, different cutoff convention.

**`f` — normalised coefficient**

`ct f signal` — cutoff `ct` is a coefficient 0.0–0.95. Approximate mapping: 0.05 ≈ 350 Hz, 0.1 ≈ 700 Hz, 0.2 ≈ 1.4 kHz, 0.4 ≈ 3 kHz, 0.7 ≈ 6.5 kHz.

Optional resonance as second parameter: `0.2 1.5 f signal`. Resonance 0–3.9. Note: the resonance feedback is from the lowpass tap rather than the bandpass tap, so it produces a broad shelf boost near cutoff rather than a sharp resonant peak — stable and musical, not Moog-style self-oscillation.

**`g` — Hz input**

`freq_hz g signal` — same filter, cutoff in Hz directly. Optional resonance: `800 2.0 g signal`. Accepts a modulation vector for swept cutoff:

```
N: 44100
T: !N
/ LFO sweeping cutoff 200–1200 Hz at 3 Hz
L: 700+(500*s +\(N#(3*(p2%N))))
W: w L g r T
```

**Highpass and bandpass**

Highpass — subtract the lowpass from the signal. Clean at zero resonance. With resonance, `signal - L` develops a shelf artefact near cutoff — usable but not a true resonant highpass:

```
N: 44100
T: !N
R: r T
L: 0.1 f R
W: w R-L
```

Bandpass — subtract two lowpass filters at different cutoffs. Works correctly at any resonance:

```
N: 44100
T: !N
R: r T
H: 0.4 f R      / lowpass ~3 kHz
L: 0.05 f R     / lowpass ~350 Hz
W: w H-L        / band between them
```

Use `f` when working with normalised coefficients. Use `g` when thinking in Hz.

### noise and percussion

`r T` — white noise. `m T` — 1-bit metallic noise, good for cymbals.

```
/ kick: pitch-swept sine + noise transient
N: 13230
T: !N
F: 50+91*e(T*(0-60%N))
P: +\(N#(F*(p2%p0)))
S: (s P)*e(T*(0-6.9%N))
R: 0.5 f r T
E: e(T*(0-40%N))
W: w (S+(R*E))
```

### patterns

The `,` operator concatenates vectors. A bar of drums is individual voice vectors joined in sequence:

```
Z: K,K,S,K,K,S,K,K,S,C,C,C,O,S,S
```

### feedback delay

`[d g] y signal` — feedback delay of `d` samples with gain `g`. Each output sample is `signal[i] + g * output[i-d]`. The output is the same length as the input.

**Pitched metallic noise** — white noise through a comb filter resonates strongly at the frequency matching the delay period and its harmonics. Delay of `SR/freq` samples tunes the resonance:

```
N: 44100
T: !N
R: r T
W: w 100 0.9 y R      / comb resonance at ~441 Hz
```

Change `100` to `200` for ~220 Hz, `50` for ~882 Hz. Higher gain = stronger resonance and more metallic character.

**Echo on a decaying sound** — the echo is only audible as a distinct repeat when the source has decayed before the delayed copy arrives. A bell with a 300ms echo:

```
N: 88200
T: !N
C: p2%p0
A: e(T*(0-4%N))
I: 3.5*e(T*(0-40%N))
P: +\(N#(440*C))
Q: +\(N#(440*C))
S: A*(s P+(I*s Q))
W: w 13230 0.5 y S    / 300ms echo at 50% level
```

**Resonant frequency boost** — when the delay exactly matches the period of the input frequency, each feedback cycle arrives perfectly in phase and the amplitude builds dramatically:

```
N: 44100
T: !N
C: p2%p0
S: s +\(N#(220*C))
W: w 200 0.9 y S      / delay=200 = one period of 220 Hz, strong resonance
```

### stereo interleave

`A z B` — interleaves two vectors into a stereo stream: `[a0, b0, a1, b1, ...]`. Output length is `min(A.length, B.length) * 2`. Useful for producing stereo output from two separately synthesised channels.

**Two voices panned left and right:**

```
N: 44100
T: !N
C: p2%p0
/ left: bell at 440 Hz
A: e(T*(0-3%N))
I: 3.5*e(T*(0-40%N))
P: +\(N#(440*C))
Q: +\(N#(440*C))
L: A*(s P+(I*s Q))
/ right: bell at 445 Hz (slight detune for stereo width)
B: e(T*(0-3%N))
J: 3.5*e(T*(0-40%N))
U: +\(N#(445*C))
V: +\(N#(445*C))
R: B*(s U+(J*s V))
/ interleave into stereo
W: w L z R
```

The resulting buffer has stereo sample pairs. Whether it plays back correctly as stereo depends on the player — the ksynth web app plays mono, so both channels will be summed.

---

## inspect commands

In the editor or REPL, type and press Enter:

| Command | Action |
|---------|--------|
| `\pV` | Play variable `V` scaled to audio levels |
| `\vV` | Graph variable `V` — min, max, zero line, length |

---

## build

```sh
# WebAssembly (requires Emscripten)
source /path/to/emsdk/emsdk_env.sh
bash build.sh

# Headless C binary
gcc -O2 ksynth.c ks_api.c -lm -o ksynth
```

Serve with `python3 -m http.server 8080` and open `http://localhost:8080`.

---

## web interface

- **16 slots** — bank any evaluated buffer to a slot; click to play; right-click for tuning and WAV export
- **notebook** — append-only run log with waveforms; collapse/expand; `→ edit` copies back to editor
- **pad panel** — 4×4 grid, drum or melodic preset, per-pad slot and pitch assignment
- **REPL strip** — persistent single-line calculator below the editor
- **session save/load** — `.json` format compatible with ksynth-desktop
- **patches browser** — load `.ks` files directly from this repo
