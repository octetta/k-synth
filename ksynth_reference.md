# ksynth Language Reference

## Overview

ksynth is a line-oriented array DSL for audio synthesis, inspired by K/APL. Each line is one expression. Variables are single uppercase letters A–Z. All values are double-precision floating-point vectors (or scalars, which are 1-element vectors). The interpreter is **right-associative**: `a op b op c` = `a op (b op c)`.

## Build

```sh
gcc -O2 ksynth.c -lm -o ksynth
```

Output is stored in variable `W`. To use as a library, include `ksynth.h` and call `e(char **s)` per line.

---

## Variables

- Single uppercase letter `A`–`Z`
- Assignment: `X: expr` evaluates `expr` and stores in `X`
- No multi-letter names, no digit suffixes (`F1`, `NE`, etc. are invalid)
- `W` is conventional output; `w W` normalizes to peak ±1

---

## Literals

```
42        scalar 42
3.14      scalar 3.14
.5        scalar 0.5   ← leading dot OK
-.25      scalar -0.25
1 2 3     vector [1, 2, 3]
1 .5 .25  vector with leading-dot elements
```

---

## Operators (dyadic, right-associative)

| Op | Name | Notes |
|----|------|-------|
| `+` | add | |
| `-` | subtract | |
| `*` | multiply | |
| `%` | divide | |
| `^` | power | |
| `&` | min | |
| `\|` | max | |
| `<` `>` `=` | compare | returns 0 or 1 |
| `,` | concatenate | |
| `#` | tile | `N#V` — tile V to length N |
| `o` | additive equal-amp | `P o H` — Σ sin(P×h) for h in H |
| `$` | additive weighted | `P $ A` — Σ A[j]×sin(P×(j+1)) |
| `f` | lowpass filter | `ct f sig` — 2-pole LP |
| `y` | feedback delay/comb | `[d g] y sig` |
| `z` | convolution | `A z B` |

### Right-associativity and linear mixes

Without parens, `A*x+B*y` parses as `A*(x+(B*y))` — **not** a linear mix.

```
/ WRONG — nested multiplication
W: S*.3+U*.7+V*.2

/ CORRECT — explicit parens
W: (S*.3)+(U*.7)+(V*.2)
```

---

## Monadic verbs (prefix)

| Verb | Name | Behavior |
|------|------|----------|
| `!N` | iota | `[0, 1, ..., N-1]` |
| `s` | sine | `sin(x)` elementwise |
| `c` | cosine | `cos(x)` elementwise |
| `e` | exp | `exp(x)` elementwise |
| `l` | log | `log(x)` elementwise |
| `a` | abs | `|x|` elementwise |
| `q` | sqrt | `sqrt(x)` elementwise |
| `h` | tanh | `tanh(x)` elementwise |
| `d` | soft clip | `tanh(3x)` elementwise |
| `x` | exp-env | `exp(-5x)` elementwise |
| `r` | white noise | uniform `[-1, 1]` per element |
| `m` | 1-bit noise | `±0.7` per element (metallic timbre) |
| `b` | negate | elementwise |
| `n` | clip to 0 | `max(0, x)` |
| `i` | integer | floor elementwise |
| `w` | normalize | scale to peak ±1.0 |
| `p` | print | print and pass through |
| `+\` | scan sum | cumulative sum (running phase) |
| `|\` | scan max | running maximum |

> **Critical:** `r` is element-wise. `r T` where `T: !N` generates N noise samples. `r N` where N is a scalar generates **1 sample**.

---

## Filter verb `f`

```
ct f signal
[ct resonance] f signal
```

2-pole lowpass. `ct` is the per-sample coefficient; cutoff frequency at 44100 Hz sr:

| `ct` | `fc` |
|------|------|
| 0.04 | ~281 Hz |
| 0.08 | ~561 Hz |
| 0.114 | ~800 Hz |
| 0.15 | ~1053 Hz |
| 0.256 | ~1800 Hz |
| 0.3 | ~2106 Hz |
| 0.5 | ~3509 Hz |
| 0.7 | ~4913 Hz |
| 0.9 | ~6317 Hz |

Formula: `fc ≈ ct × sr / (2π)`

**Highpass:** `sig - (ct f sig)`

**Bandpass 800–1800 Hz:**
```
A: 0.256 f R    / LP at 1800Hz
B: 0.114 f R    / LP at 800Hz
C: A-B          / band between them
```

---

## Weighted additive synthesis `$`

```
phase_vector $ amplitude_vector
```

`result[i] = Σ_j amp[j] × sin(phase[i] × (j+1))`

Index 0 = harmonic 1, index 1 = harmonic 2, etc. Use 0 for absent harmonics.

```
/ sawtooth (32 harmonics, 1/h amplitudes)
H: !32
H: H+1
A: 1%H
W: w P $ A

/ organ drawbar (h1,2,3,4,6,8 equal, rest silent)
A: 1 1 1 1 0 1 0 1
W: w P $ A

/ cowbell — slightly square character
A: 1 0 0.3 0 0.15
W: w P $ A
```

Versus `o` (equal-amplitude additive):
- `P o (1 3 5 7)` — sum at those harmonic numbers, equal amplitude
- `P $ A` — sum at harmonics 1,2,3,... with amplitudes from A

---

## Standard synthesis patterns

### Envelope (exponential decay)
```
N: 44100
T: !N
E: e(T*(0-6.9%N))    / k=6.9 → −60 dB at end of buffer
```
For faster decays use larger k. Tau in samples = `N/k`.

### Pitch-sweeping oscillator
```
N: 13230
T: !N
F: 50+91*e(T*(0-60%N))    / 141Hz → 50Hz sweep
D: F*(6.28318%44100)       / phase increment per sample
P: +\D                     / cumulative phase
S: s P                     / oscillator output
```

### Fixed-frequency oscillator
```
F: 440*(6.28318%44100)
P: +\(N#F)
S: (s P)                   / parens prevent FM misparse
```

> **FM hazard:** `s P + s Q` parses as `s(P + s(Q))` — FM synthesis, not a mix!
> Use `(s P) + (s Q)` or assign each sine to a variable first.

### Noise generation
```
T: !N
R: r T          / N samples white noise
M: m T          / N samples 1-bit noise (metallic)
```

### Highpass filter
```
R: r T
L: 0.5 f R      / lowpass at ~3500 Hz
H: R-L          / highpass: subtract LP
```

### Rise-then-fall envelope (staggered bursts)
```
/ T*e(−T*k/N) peaks at sample N/k
/ k=30 → peak at 7ms  (N=8820, sr=44100)
/ k=15 → peak at 13ms
/ k=8  → peak at 25ms
T: !N
X: T*e(T*(0-8%N))
E: w X              / normalize envelope to peak=1
```
Used for 808 clap: three independent burst layers with staggered peak times.

---

## 808 Drum Kit

Synthesized from real 808 samples (measured spectra and decay envelopes).

| File | Voice | Key freq | Duration | Method |
|------|-------|----------|----------|--------|
| `drums-kick.ks` | Bass Drum | 141→50 Hz | 300ms | Pitch-sweep oscillator |
| `drums-snare.ks` | Snare | 170 Hz body | 280ms | Sines + noise tail |
| `drums-clap.ks` | Clap | 800–1800 Hz | 200ms | 3 staggered bandpassed bursts |
| `drums-chh.ks` | Closed Hi-Hat | >6 kHz | 200ms | 1-bit noise + HP |
| `drums-ohh.ks` | Open Hi-Hat | >6 kHz | 280ms | Same as CHH, longer |
| `drums-hitom.ks` | Hi Tom | 170 Hz | 500ms | Pitch-sweep |
| `drums-midtom.ks` | Mid Tom | 122 Hz | 500ms | Pitch-sweep |
| `drums-lotom.ks` | Lo Tom | 80 Hz | 640ms | Pitch-sweep |
| `drums-rim.ks` | Rimshot | 1800 Hz | 60ms | Short tonal burst + HP noise |
| `drums-cowbell.ks` | Cowbell | 735+850 Hz | 900ms | Two tones, `$` spectrum |
| `drums-crash.ks` | Crash | 3 kHz inharmonic | 1500ms | 5 inharmonic oscillators + 1-bit noise |
| `drums-clave.ks` | Clave | 2500 Hz | 100ms | Two detuned sines |
| `drums-maracas.ks` | Maracas | 4 kHz | 30ms | 1-bit noise + HP |
| `drums-trigger.ks` | Trigger Out | 100 Hz | 650ms | Fixed-pitch sine |

---

## DW-8000 Wavetables

Files `dw8k-01.ks` through `dw8k-16.ks`. Amplitudes measured via DFT from ROM data.

| # | Name | Character |
|---|------|-----------|
| 01 | strings | Perfect 1/h sawtooth |
| 02 | clarinet | Odd harmonics, 1/h |
| 03 | apiano | h2>h1, bumps h8–h14 |
| 04 | epiano | Similar to apiano |
| 05 | epiano-hard | Brighter |
| 06 | clavi | Complex — h3≈h7≈h1, alternating phase |
| 07 | organ | Drawbar h1,2,3,4,6,8,10,12,16 equal |
| 08 | brass | h2≈h3≈h1, very bright |
| 09 | sax | Strong h2–h5 |
| 10 | violin | Formant clusters at h14, h22–23 |
| 11 | aguitar | Similar to violin |
| 12 | dguitar | Picking transient emphasis |
| 13 | ebass | Low-end focused |
| 14 | dbass | Same pattern as apiano |
| 15 | bell | Sparse every-4th harmonics; h17 > h1 |
| 16 | whistle | Near-pure sine (h1=0.989) |

---

## Parser quirks summary

| Issue | Symptom | Fix |
|-------|---------|-----|
| Right-assoc mix | `A*.5+B*.3` = `A*(0.5+B*0.3)` | `(A*.5)+(B*.3)` |
| FM hazard | `s P+Q` = FM, not mix | `(s P)+(s Q)` |
| Noise on scalar | `r N` = 1 sample | `r T` where `T: !N` |
| Negation | `-X` ambiguous | Use `0-X` for expressions |

---

## Test suite

```sh
gcc -O2 test_ksynth.c ksynth.c -lm -o test_ksynth && ./test_ksynth
# 151 tests, 0 failures
```

Coverage: scalars · vectors · arithmetic · scan/reduce · normalize · sine/cosine · exp/log · filter cutoff · highpass subtract · delay/comb · additive synthesis `o` and `$` · dot literals · right-assoc mix correctness · envelope decay · noise length · pitch sweep · rise-decay envelope · 1-bit noise
