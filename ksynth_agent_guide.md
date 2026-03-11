# ksynth — Agent Guide

ksynth is a compact line-oriented DSL for audio synthesis. It processes one expression per line, accumulates results in single-letter variables, and outputs a wavetable in variable `W`. This guide is written for an AI agent generating `.ks` programs.

---

## Execution model

- One expression per line. Lines execute in order.
- Variables are single uppercase letters `A`–`Z`. No exceptions.
- All values are `double` vectors. A scalar is a 1-element vector.
- The interpreter is **right-associative** — this is the most important thing to internalize.
- Comments begin with `/` and run to end of line.
- Output convention: store final audio in `W`. Use `W: w expr` to normalize to peak ±1.

```
/ minimal program
N: 4096
T: !N
P: ~N
W: w s P
```

---

## Variables — hard rules

```
/ VALID
A: 42
T: !N
F: 440*(6.28318%44100)

/ INVALID — will silently fail or misbehave
AB: 42          / multi-letter — forbidden
F1: 440         / digit suffix — forbidden
```

You have 26 variables. Track usage on complex programs — the 808 cymbal uses ~17.

---

## Literals

```
42          scalar
3.14        scalar
.5          0.5    ← leading dot is valid
-.25        -0.25
1 2 3       vector [1, 2, 3]
1 .5 .25    vector [1.0, 0.5, 0.25]
```

---

## Right-associativity — the central hazard

Every operator chains right: `A op B op C` = `A op (B op C)`.

This is correct for most uses but **breaks linear mixes**:

```
/ WRONG: parses as S * (0.3 + U * (0.7 + V * 0.2))
W: w S*.3+U*.7+V*.2

/ CORRECT: explicit parens around each scaled term
W: w (S*.3)+(U*.7)+(V*.2)
```

**Rule: whenever you mix multiple sources with `+` and `*`, parenthesize every term.**

This also affects the `s` verb:

```
/ WRONG: FM synthesis — s(P + s(Q))
S: s P+s Q

/ CORRECT: mix of two sines
S: (s P)+(s Q)
```

**Rule: always parenthesize `(s X)` before using `+` or `*` on the result.**

---

## All verbs

### Monadic (prefix, takes one argument to the right)

| Verb | Returns | Notes |
|------|---------|-------|
| `!N` | `[0,1,...,N-1]` | iota — fundamental index vector |
| `~N` | `[0, 2π/N, ..., 2π(N-1)/N]` | phase ramp over one cycle |
| `+V` | scalar sum | reduce: sum all elements |
| `>V` | scalar peak | reduce: max absolute value |
| `w V` | normalized vector | scale to peak ±1.0 |
| `s V` | sin(v) | elementwise |
| `c V` | cos(v) | elementwise |
| `t V` | tan(v) | elementwise |
| `h V` | tanh(v) | elementwise — soft limit |
| `d V` | tanh(3v) | elementwise — harder soft clip |
| `a V` | \|v\| | elementwise absolute value |
| `q V` | sqrt(\|v\|) | elementwise |
| `l V` | log(\|v\|) | elementwise, log(v+ε) safe |
| `e V` | exp(v) | elementwise — use for envelopes |
| `x V` | exp(-5v) | elementwise — fast exp-env shape |
| `r V` | noise | uniform [-1,1], **one sample per input element** |
| `m V` | 1-bit noise | ±0.7, deterministic hash, metallic timbre |
| `b V` | band noise | sum of 6 inharmonic square waves |
| `u V` | attack ramp | 0→1 over first 10 samples, then 1.0 |
| `n V` | MIDI→Hz | 440 × 2^((v-69)/12) |
| `i V` | reverse | reverses vector (not floor!) |
| `_ V` | floor | elementwise floor to integer |
| `p V` | print+pass | debug: prints and returns input |
| `v V` | quantize/4 | round to nearest 0.25 step |
| `j V` | left channel | extract even-indexed samples from interleaved stereo |
| `k V` | right channel | extract odd-indexed samples |
| `+\V` | scan sum | cumulative sum — use for phase accumulation |
| `|\V` | scan max | running maximum |

### Dyadic (infix, `A verb B`)

| Verb | Operation | Notes |
|------|-----------|-------|
| `+` `-` `*` `%` | arithmetic | `%` is divide |
| `^` | power | `A^2` |
| `&` | min | elementwise |
| `\|` | max | elementwise |
| `<` `>` `=` | compare | returns 0.0 or 1.0 |
| `,` | concatenate | `A,B` — append B after A |
| `#` | tile | `N#V` — tile V to length N (scalar N) |
| `o` | additive equal-amp | `P o H` — Σ sin(P×h) for h in H |
| `$` | additive weighted | `P $ A` — Σ A[j]×sin(P×(j+1)) |
| `f` | lowpass filter | `ct f sig` — 2-pole LP, ct in (0, 0.95) |
| `y` | feedback delay | `[d g] y sig` — comb filter, d=delay samples, g=gain |
| `z` | stereo interleave | `L z R` — interleave two mono vectors |
| `v` | quantize/N | `N v sig` — quantize to N levels |

---

## Core synthesis patterns

### 1. Index vector and phase ramp

Always start with `T: !N` (sample indices) and build from there.

```
N: 4096       / buffer length in samples
T: !N         / [0, 1, 2, ..., 4095]
```

For a one-cycle wavetable, `~N` gives the phase directly:
```
P: ~N         / [0, 2π/N, ..., 2π(N-1)/N]
W: w s P      / one cycle of sine
```

### 2. Oscillator at fixed frequency

```
N: 44100          / 1 second at 44100 Hz
T: !N
F: 440*(6.28318%44100)   / phase increment per sample for 440 Hz
P: +\(N#F)               / N#F tiles scalar to vector; +\ cumulates
W: w (s P)               / parentheses prevent FM misparse
```

The formula `freq*(6.28318%sr)` gives radians-per-sample. Always compute it this way.

### 3. Pitch-sweeping oscillator

```
N: 13230
T: !N
/ frequency decays from (base+range) to base
F: 50+91*e(T*(0-60%N))   / 141Hz → 50Hz
D: F*(6.28318%44100)
P: +\D
S: (s P)
```

`e(T*(0-k%N))` gives `exp(-k*t/N)` — exponential decay from 1.0 to `exp(-k)`.
At `k=6.9`, the value reaches ~0.001 (−60 dB) at `t=N`.

### 4. Exponential envelope

```
N: 44100
T: !N
E: e(T*(0-6.9%N))    / full decay over buffer
```

| k value | -60 dB reached at | Tau |
|---------|-------------------|-----|
| 6.9 | end of buffer (t=N) | N/6.9 samples |
| 30 | N/30 | fast attack/decay |
| 80 | N/80 | very fast snap |
| 200 | N/200 | near-instant |

Tau in samples = `N/k`. Tau in ms = `N/(k*sr) * 1000`.

**Negation:** always write `(0-k%N)`, not `(-k%N)` or `-k%N`, to avoid parse ambiguity.

### 5. Rise-then-fall envelope

`T * e(-T*k/N)` rises from 0, peaks at sample `N/k`, then decays.

```
N: 8820
T: !N
X: T*e(T*(0-8%N))    / peaks at N/8 = 1102 samples = 25ms
E: w X               / normalize peak to 1.0
```

Use this for staggered drum bursts (e.g., 808 clap layers).

### 6. Noise sources

```
T: !N
R: r T     / white noise [-1,1], N samples
M: m T     / 1-bit noise ±0.7, N samples, deterministic metallic character
```

**Critical:** `r` is element-wise. `r N` where N is a scalar gives **one** noise sample. Always use `r T` where `T: !N`.

### 7. Lowpass filter

```
ct f signal
```

`ct` is the coefficient. Cutoff frequency: `fc ≈ ct × sr / (2π)`.

| `ct` | Cutoff at 44100 Hz sr |
|------|----------------------|
| 0.04 | 281 Hz |
| 0.08 | 561 Hz |
| 0.114 | 800 Hz |
| 0.15 | 1053 Hz |
| 0.256 | 1800 Hz |
| 0.3 | 2106 Hz |
| 0.5 | 3509 Hz |
| 0.7 | 4913 Hz |
| 0.9 | 6317 Hz |

With resonance (second element of left arg, 0–3.98):
```
0.3 0.5 f R     / LP at 2106Hz with moderate resonance
```

### 8. Highpass filter

ksynth has no native HP. Subtract a lowpass:

```
R: r T
L: 0.15 f R       / LP at 1053 Hz
H: R-L            / HP above 1053 Hz
```

The filter attenuates -12 dB/octave below the cutoff. Not steep — use high `ct` values (0.5–0.8) to push the cutoff high enough for bright sounds.

### 9. Bandpass filter

```
R: r T
A: 0.256 f R      / LP at 1800 Hz
B: 0.114 f R      / LP at 800 Hz
C: A-B            / bandpass: 800–1800 Hz
```

The order matters: `A` (higher cutoff, more content) minus `B` (lower cutoff, less content) = the band between them.

### 10. Weighted additive synthesis

```
P $ A
```

`result[i] = Σ_j A[j] × sin(P[i] × (j+1))`

`A[0]` = amplitude of harmonic 1, `A[1]` = harmonic 2, etc. Use 0 to skip harmonics.

```
/ sawtooth
H: !32
H: H+1        / [1, 2, 3, ..., 32]
A: 1%H        / [1, 0.5, 0.333, ..., 0.03125]
W: w P $ A

/ square wave (odd harmonics only)
A: 1 0 0.333 0 0.2 0 0.143 0 0.111
W: w P $ A

/ organ (drawbars 1,2,3,4,6,8,10,12,16 equal)
A: 1 1 1 1 0 1 0 1 0 1 0 1 0 0 0 1
W: w P $ A

/ bell-like (inharmonic via non-integer phase scaling)
F: 1.0*(6.28318%44100)
G: 2.756*(6.28318%44100)     / inharmonic ratio
P: +\(N#F)
Q: +\(N#G)
W: w (s P)+(s Q)*.6
```

For `o` (equal amplitude, explicit harmonic numbers):
```
W: w P o (1 3 5 7 9)    / odd harmonics, equal amplitude
```

Use `o` when choosing which harmonics are present. Use `$` when controlling their amplitudes.

### 11. Linear mix — the correct form

Every time you combine scaled vectors, parenthesize:

```
/ Three layers mixed linearly
W: w (A*.5)+(B*.3)+(C*.2)

/ Two sources with individual envelopes
W: w (E1*S1)+(E2*S2)
```

Without parens: `E1*S1+E2*S2` = `E1*(S1+(E2*S2))` — S1 and S2 are entangled.

### 12. Stereo output

```
L: w left_signal
R: w right_signal
W: L z R      / interleave into stereo
```

Extract channels from an interleaved vector: `j` = left (even samples), `k` = right (odd samples).

---

## Drum synthesis recipes

### Kick drum (808-style)

```
N: 13230          / 300ms
T: !N
E: e(T*(0-6.9%N))
/ pitch sweep: start freq → end freq
F: 50+91*e(T*(0-60%N))    / 141Hz → 50Hz
D: F*(6.28318%44100)
P: +\D
S: (s P)
/ short sub-bass thump at attack
Q: e(T*(0-300%N))
R: r T
C: 0.04 f R               / LP at 281Hz — sub-only noise
W: w (E*S)+(Q*C*.1)
```

Key parameters: start frequency (determines pitch character), end frequency (the "note" you hear sustained), sweep speed (k in pitch envelope), overall decay (k=6.9 for full buffer).

### Snare drum

```
N: 12348          / 280ms
T: !N
/ body: two detuned sines, fast decay
B: e(T*(0-25%N))
F: 170*(6.28318%44100)
G: 183*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
S: B*(s P+s Q)*.5     / NOTE: s P+s Q is FM here — body freq is being modulated
/ For clean mixing, assign sines separately:
/ SP: (s P)
/ SQ: (s Q)
/ S: B*((SP+SQ)*.5)
E: e(T*(0-6.9%N))
R: r T
U: E*R
V: e(T*(0-80%N))
K: r T
/ body dominates spectrum, noise provides rattle
W: w (S*.7)+(U*.4)+(V*K*.2)
```

Body frequency: 150–200 Hz for 808-style. Higher = tighter snare. Noise tail length is controlled by the buffer N and k=6.9.

### Hi-hat (closed)

```
N: 8820           / 200ms
T: !N
E: e(T*(0-6.9%N))
M: m T            / 1-bit noise: naturally bright and metallic
R: r T
L: 0.5 f R        / LP at 3509 Hz
H: R-L            / HP: removes lows from white noise
W: w E*(M*.6+H*.4)
```

Open hi-hat: same but longer buffer (N=12348 or more). The `m` verb is the key to metallic hi-hat timbre — it produces a deterministic hash-based ±0.7 pattern that sounds distinctly metallic versus white noise.

### Clap (808-style — staggered burst)

The 808 clap has 3–4 noise bursts arriving ~10ms apart, each bandpassed to 800–1800 Hz, with the last burst loudest. Approximate with rise-then-fall envelopes peaking at different times:

```
N: 8820           / 200ms
T: !N

/ burst 1: peaks at ~7ms (k=30 → N/30 = 294 samples)
R: r T
A: 0.256 f R      / LP at 1800Hz
B: 0.114 f R      / LP at 800Hz
C: A-B            / bandpass
X: w (T*e(T*(0-30%N)))

/ burst 2: peaks at ~13ms (k=15 → N/15 = 588 samples)
J: r T
D: 0.256 f J
F: 0.114 f J
G: D-F
Y: w (T*e(T*(0-15%N)))

/ burst 3 (main clap): peaks at ~25ms (k=8 → N/8 = 1102 samples)
K: r T
E: K-(0.114 f K)
H: 0.256 f E
Z: w (T*e(T*(0-6%N)))

W: w (X*C*.3)+(Y*G*.5)+Z*H
```

Note: `Z*H` at the end without parens is fine since it's the last term — there's nothing after it for right-assoc to entangle.

### Tom drum

```
N: 22050          / 500ms
T: !N
E: e(T*(0-6.9%N))
/ pitch sweep: small range, fast
/ hi tom: 212→170Hz, mid: 165→122Hz, lo: 125→80Hz
F: 170+42*e(T*(0-80%N))
D: F*(6.28318%44100)
P: +\D
S: (s P)          / parentheses required — no FM
/ tiny lowpassed thump
Q: e(T*(0-200%N))
R: r T
C: 0.1 f R
W: w (E*S)+(Q*C*.08)
```

### Cowbell / metallic tonal percussion

```
N: 39690          / 900ms
T: !N
E: e(T*(0-6.9%N))
/ two close frequencies (measured 808: 735Hz + 850Hz)
F: 735*(6.28318%44100)
G: 850*(6.28318%44100)
P: +\(N#F)
Q: +\(N#G)
/ $ adds harmonic content for slightly square character
A: 1 0 0.3 0 0.15
J: P $ A
K: Q $ A
M: J+K*.8
W: w E*M
```

### Rimshot

```
N: 2646           / 60ms — rim is very short
T: !N
E: e(T*(0-6.9%N))
F: 1800*(6.28318%44100)   / 1800Hz dominant
P: +\(N#F)
S: (s P)
R: r T
L: 0.15 f R
H: R-L            / HP noise for broadband crack
W: w E*(S*.6+H*.5)
```

### Crash cymbal (inharmonic oscillators)

```
N: 66150          / 1500ms
T: !N
E: e(T*(0-6.9%N))
/ inharmonic frequency ratios (standard cymbal ratios × base)
B: 3000*(6.28318%44100)
P: +\(N#(B*1.000))
Q: +\(N#(B*1.342))
R: +\(N#(B*1.200))
S: +\(N#(B*1.618))
U: +\(N#(B*1.478))
/ odd-harmonic content from $
A: 1 0 0.5 0 0.25
J: P $ A
K: Q $ A
L: R $ A
M: S $ A
X: U $ A
Z: J+K+L+M+X
/ 1-bit noise shimmer
C: m T
G: e(T*(0-6.9%N))
W: w (E*Z*.7)+(G*C*.4)
```

---

## Synth sound recipes

### Classic sawtooth oscillator

```
N: 4096
P: ~N             / phase ramp 0..2π
H: !32
H: H+1
A: 1%H
W: w P $ A
```

### Square wave

```
N: 4096
P: ~N
A: 1 0 0.333 0 0.2 0 0.143 0 0.111 0 0.0909
W: w P $ A
```

### Triangle wave

```
/ Triangle = 1/h² odd harmonics, alternating sign
N: 4096
P: ~N
A: 1 0 -.111 0 .04 0 -.0204 0 .0123
W: w P $ A
```

### Organ (drawbar)

```
/ Classic B3 registration: 008080800 (approximate)
N: 4096
P: ~N
A: 0 0 0 0 0 0 0 1 0 1 0 0 0 0 0 0 1
W: w P $ A
```

Drawbar harmonic positions: the 9 drawbars correspond to harmonics 1, 2, 3, 4, 5, 6, 8, 10, 16. Map to `$` amplitude vector with zeros for unused harmonics.

### Bell / FM-style inharmonic

```
N: 4096
P: ~N
/ Two inharmonic partials — adjust ratio for different bell characters
/ Ratio 2.756 = classic bell
Q: ~N
F: 1.0
G: 2.756
/ Scale phases by respective multipliers
/ then $ adds overtone content to each partial
A: 1 0 0.3 0 0.1
W: w (P $ A)+((Q*2.756) $ (A*.5))
```

### Lowpass filtered noise (wind, breath)

```
N: 44100
T: !N
R: r T
L: 0.08 f R       / LP at 561Hz — dark wind sound
/ or: 0.3 f R for brighter noise
W: w L
```

### PWM (pulse width modulation approximation)

```
/ Sum odd harmonics with amplitude modulation
/ True PWM needs a comparator but $ can approximate
N: 4096
P: ~N
/ Narrow pulse: many odd harmonics
A: 1 0 0.9 0 0.7 0 0.5 0 0.3 0 0.1
W: w P $ A
```

### Formant / vowel synthesis

```
N: 4096
P: ~N
/ EE vowel: F1≈270Hz, F2≈2300Hz
/ For a 100Hz fundamental: h3≈270Hz, h23≈2300Hz
/ Gaussian peaks at h=3 and h=23
/ (build amplitude array with bumps at those positions)
H: !32
H: H+1
/ Gaussian centered at harmonic 3, width 2
G1: e((H-3)^2*(0-0.5))
/ Gaussian centered at harmonic 23
G2: e((H-23)^2*(0-0.5))
A: G1+G2*.6
W: w P $ A
```

---

## Anti-patterns — things that silently go wrong

### 1. Multi-letter variable names
```
/ WRONG — parser treats HP as H applied to P
HP: R-L
/ RIGHT
H: R-L
```

### 2. Noise on a scalar
```
/ WRONG — generates 1 noise sample
N: 4410
R: r N

/ RIGHT — element-wise over index vector
T: !N
R: r T
```

### 3. Unparenthesized sine before addition
```
/ WRONG — FM synthesis: s(P + s(Q)*0.3) 
S: s P+s Q*.3

/ RIGHT
SP: (s P)
SQ: (s Q)
S: SP+SQ*.3          / still right-assoc: SP + (SQ*0.3) — OK here
```

### 4. Unparenthesized mix
```
/ WRONG — not a linear blend
W: w A*.5+B*.3+C*.2

/ RIGHT
W: w (A*.5)+(B*.3)+(C*.2)
```

### 5. Negation ambiguity
```
/ WRONG — may parse as (-k) % N or miss the unary minus
E: e(T*(-k%N))

/ RIGHT — explicit subtraction from zero
E: e(T*(0-k%N))
```

### 6. FM via right-assoc in body computation
```
/ WRONG — B*(s P + s Q) parses as B*(s(P + s(Q))) = FM
S: B*(s P+s Q)*.5

/ RIGHT
SP: s P
SQ: s Q
S: B*((SP+SQ)*.5)
```

### 7. Reusing a variable you still need
```
/ WRONG — H is overwritten before use in W
H: !32
H: H+1      / fine, rebinds H
A: 1%H
H: R-L      / now H is the highpass — but if you need the harmonic H later, it's gone
```

### 8. Running out of variables
You have 26. Count before writing complex programs. Use short names for intermediates that won't be reused.

---

## Debugging tips

- `p X` prints the vector X and passes it through: `W: p (s P)` shows sine output
- `+X` returns the sum as a scalar — useful to check a vector has nonzero content
- `>X` returns the peak absolute value — check for unexpectedly large or zero signals
- Add `W: w intermediate` at any point to inspect a partial result
- If a drum voice is silent, check: (a) `r T` not `r N`, (b) envelope k values aren't so large the signal decays instantly, (c) no FM parsing of sines

---

## Filter cookbook

```
/ Remove sub-bass (<280Hz): subtract LP at 0.04
H: R-(0.04 f R)

/ Keep only lows (<560Hz): LP at 0.08
L: 0.08 f R

/ Band 560–1800Hz: LP(0.256) - LP(0.08)
A: 0.256 f R
B: 0.08 f R
C: A-B

/ Remove lows, keep mid-highs (>800Hz):
H: R-(0.114 f R)

/ Very bright only (>3500Hz):
H: R-(0.5 f R)

/ Add resonance to LP:
L: 0.15 0.8 f R      / LP at 1053Hz with resonance=0.8

/ Comb filter / echo:
/ [delay_samples, gain] y signal
E: 441 0.4 y R       / echo at 10ms, gain 0.4
```

---

## Envelope cookbook

```
/ Standard -60dB decay over buffer
E: e(T*(0-6.9%N))

/ Fast body (dies in N/30 of buffer)
B: e(T*(0-30%N))

/ Snap transient only
V: e(T*(0-80%N))

/ Near-instant click
C: e(T*(0-200%N))

/ Rise-then-fall, peak at 25ms (N=8820)
X: T*e(T*(0-8%N))
E: w X

/ Pitch envelope for sweep (higher k = faster sweep)
F: F_low + F_range*e(T*(0-60%N))
```

---

## Complete 808 drum kit overview

| File | Voice | Dominant freq | Duration | Key technique |
|------|-------|---------------|----------|---------------|
| `drums-kick.ks` | Bass Drum | 141→50 Hz | 300ms | Pitch sweep |
| `drums-snare.ks` | Snare | 170 Hz | 280ms | Pitched body + noise |
| `drums-clap.ks` | Clap | 800–1800 Hz | 200ms | 3 staggered bandpassed bursts |
| `drums-chh.ks` | Closed Hat | >6 kHz | 200ms | 1-bit noise + HP |
| `drums-ohh.ks` | Open Hat | >6 kHz | 280ms | Same, longer |
| `drums-hitom.ks` | Hi Tom | 170 Hz | 500ms | Pitch sweep, small range |
| `drums-midtom.ks` | Mid Tom | 122 Hz | 500ms | Pitch sweep |
| `drums-lotom.ks` | Lo Tom | 80 Hz | 640ms | Pitch sweep |
| `drums-rim.ks` | Rimshot | 1800 Hz | 60ms | Short tone + HP noise |
| `drums-cowbell.ks` | Cowbell | 735+850 Hz | 900ms | Two tones + `$` |
| `drums-crash.ks` | Crash | 3 kHz inharmonic | 1500ms | 5 inharmonic OSCs + 1-bit |
| `drums-clave.ks` | Clave | 2500 Hz | 100ms | Two detuned sines |
| `drums-maracas.ks` | Maracas | 4 kHz | 30ms | 1-bit noise + HP |
| `drums-trigger.ks` | Trigger | 100 Hz | 650ms | Fixed sine |
