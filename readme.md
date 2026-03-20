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

---

## quick start

```
/ sine wave, 1 second, 440 Hz
N: 44100
T: !N
W: w s +\(N#(440*(6.28318%44100)))
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
P: +\(N#(6.28318%N))
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
P: +\(N#(6.28318%N))
I: 2.5
T: s P+(I*s P)
M: 69
D: 88200
W: w T t (nM) D
```

Monadic `t` remains `tan`.

### oscillators

The phase accumulator pattern `+\(N#F)` where `F` is a per-sample phase increment gives a clean oscillator at any frequency. Apply `s` for sine, `c` for cosine, or do math on the raw ramp for triangle and sawtooth.

```
/ sawtooth at 220 Hz, 1 second (via harmonic sum)
N: 44100
T: !N
F: 220*(6.28318%44100)
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
C: 440*(6.28318%44100)
M: 440*(6.28318%44100)
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
P: +\(N#(440*(6.28318%44100)))
W: w A*s P
```

Adjust the `3` to taste — larger decays faster, smaller lingers longer. At `k=1` the decay is very slow; at `k=10` it's a short pluck.

**Percussive rise-and-fall** — a thump that swells briefly then fades:

```
N: 44100
T: !N
A: T*e(T*(0-8%N))
P: +\(N#(180*(6.28318%44100)))
W: w A*s P
```

The peak lands at sample `N/k` — here `44100/8` ≈ 5500 samples ≈ 125ms in. Good for kick and tom shapes.

**Two envelopes on one voice** — fast index decay for a bright attack, slow amplitude decay for the body (the FM bell pattern):

```
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

`A` decays slowly (the ring). `I` decays fast (the clang). The combination is what makes it sound like a struck bell rather than a plain FM tone.

**Soft clipping** — `d` applies `tanh(3x)`, rounding peaks without hard discontinuities. Useful after loud envelopes:

```
N: 44100
T: !N
A: T*e(T*(0-5%N))
P: +\(N#(220*(6.28318%44100)))
W: w d A*s P
```

### filters

Two Chamberlin state-variable lowpass filters — same topology, different cutoff convention.

**`f` — normalised coefficient**

`ct f signal` — cutoff `ct` is a coefficient 0.0–0.95. Approximate mapping: 0.05 ≈ 350 Hz, 0.1 ≈ 700 Hz, 0.2 ≈ 1.4 kHz, 0.4 ≈ 3 kHz, 0.7 ≈ 6.5 kHz. Optional resonance as second parameter: `0.2 1.5 f signal`. Resonance 0–3.9; above ~3.5 approaches self-oscillation. A cutoff vector the same length as the signal gives a time-varying filter.

Subtract the lowpass from the original for highpass. Subtract two lowpass results for bandpass.

**`g` — Hz input**

`freq_hz g signal` — same filter, cutoff in Hz directly. Optional resonance: `800 2.0 g signal`. Accepts a modulation vector for swept cutoff:

```
N: 44100
T: !N
/ LFO sweeping cutoff 200–1200 Hz at 3 Hz
L: 700+(500*s +\(N#(3*(6.28318%N))))
W: w L g r T
```

Use `f` when working with normalised coefficients. Use `g` when thinking in Hz.

### noise and percussion

`r T` — white noise. `m T` — 1-bit metallic noise, good for cymbals.

```
/ kick: pitch-swept sine + noise transient
N: 13230
T: !N
F: 50+91*e(T*(0-60%N))
P: +\(N#(F*(6.28318%44100)))
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

`[d g] y signal` — feedback delay of `d` samples with gain `g`. Values near 1.0 give resonant comb filtering.

### convolution

`A z B` — convolves two vectors. Design FIR filters or apply impulse responses.

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
