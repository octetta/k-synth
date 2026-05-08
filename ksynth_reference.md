# ksynth language reference

ksynth is a line-oriented array DSL for audio synthesis, inspired by K/APL. Each line is one expression. Variables are single uppercase letters `A`–`Z`. All values are double-precision floating-point vectors — scalars are 1-element vectors. The interpreter is right-associative: `a op b op c` = `a op (b op c)`.

---

## variables and assignment

- Single uppercase letter `A`–`Z` only. No multi-letter names, no digit suffixes.
- `X: expr` evaluates `expr` and stores in `X`.
- `W` is the conventional output — every script sets it via `W: w expr`.
- Inside a function body, `x` and `y` are the first and second arguments.

---

## literals

```
42          scalar 42
3.14        scalar 3.14
.5          scalar 0.5    / leading dot valid
1 2 3       vector [1, 2, 3]
1 0.5 0.25  vector — space-separated
```

Negative numbers: a spaced minus is negation (continues the vector); a flush minus is subtraction (ends the vector):

```
1 -2 3      vector [1, -2, 3]
A-1         A minus 1 (subtraction)
0-X         negate X (use this form in expressions, not -X)
```

---

## monadic verbs

| Verb | Behavior |
|------|----------|
| `!N` | iota: `[0, 1, ..., N-1]` |
| `~N` | phase ramp: `[0, 2π/N, ..., 2π*(N-1)/N]` |
| `s V` | sin elementwise |
| `c V` | cos elementwise |
| `t V` | tan elementwise |
| `h V` | tanh(V) — soft saturation |
| `d V` | tanh(3V) — harder soft clip |
| `a V` | abs elementwise |
| `q V` | sqrt(abs(V)) elementwise |
| `l V` | log(abs(V)+ε) elementwise |
| `e V` | exp(V), input clamped to [-100, 100] |
| `x V` | exp(-5V) — fast decay shape |
| `_ V` | floor elementwise |
| `p V` | 44100 if V=0, else π×V elementwise |
| `n V` | MIDI note to Hz: `440 * 2^((V-69)/12)` |
| `i V` | reverse vector |
| `j V` | extract left channel (even samples) from interleaved stereo |
| `k V` | extract right channel (odd samples) from interleaved stereo |
| `r V` | white noise: uniform [-1,1], one sample per element of V |
| `m V` | 1-bit metallic noise: deterministic ±0.7 per element |
| `b V` | band-limited buzz at 110 Hz (monadic default) |
| `u V` | anti-click ramp: 0→1 over first 10 samples, then 1.0 |
| `v V` | quantize to 4 levels (nearest 0.25) |
| `w V` | peak-normalize to ±1.0 — use for output |
| `+V` | sum all elements (scalar result) |
| `>V` | peak absolute value (scalar result) |

`r` and `m` use only the length of V, not its values. `b` and `u` likewise.

`n`, `j`, `k`, `v`: see dyadic forms below for additional arguments.

---

## dyadic verbs

All dyadic verbs are right-associative. Length of result = max of inputs; the shorter side cycles.

### arithmetic

| Op | Behavior |
|----|----------|
| `A + B` | add |
| `A - B` | subtract |
| `A * B` | multiply |
| `A % B` | divide (`%` is division, not modulo) |
| `A ^ B` | power: `abs(A)^B` |
| `A & B` | min elementwise |
| `A \| B` | max elementwise |
| `A < B` | 1.0 if A<B else 0.0 |
| `A > B` | 1.0 if A>B else 0.0 |
| `A = B` | 1.0 if A=B else 0.0 |
| `N # V` | tile V to length N (cyclic) |
| `A , B` | concatenate |

`%` is **division**. Fractional part of X: `X - _(X)`.

Hard clip requires explicit parens — `&` and `|` are right-associative:

```
/ hard clip to [-0.5, 0.5]
C: (S & 0.5) | -0.5   / correct
C: S & -0.5 | 0.5     / wrong — parses as S & (-0.5 | 0.5) = S & 0.5
```

### filters

| Verb | Usage | Description |
|------|-------|-------------|
| `f` | `ct f signal` | 2-pole lowpass; `ct` = normalised coefficient 0–0.95 |
| `f` | `ct rs f signal` | with resonance `rs` 0–3.9 |
| `g` | `hz g signal` | 2-pole lowpass, cutoff in Hz |
| `g` | `hz q g signal` | with Q 0.01–3.9 |

Approximate `f` coefficient mapping (44100 Hz):

| `ct` | `fc` |
|------|------|
| 0.04 | ~281 Hz |
| 0.08 | ~561 Hz |
| 0.11 | ~800 Hz |
| 0.15 | ~1053 Hz |
| 0.26 | ~1800 Hz |
| 0.5  | ~3509 Hz |
| 0.7  | ~4913 Hz |
| 0.9  | ~6317 Hz |

Highpass: subtract the lowpass. Bandpass: subtract two lowpass filters at different cutoffs.

```
H: R - (0.1 f R)          / highpass
B: (0.4 f R)-(0.05 f R)   / bandpass
```

`g` accepts a modulation vector for swept cutoff: if `hz` has the same length as `signal`, each sample uses its own cutoff value.

### feedback delay

| Verb | Usage | Description |
|------|-------|-------------|
| `y` | `d g y signal` | feedback delay: `out[i] = signal[i] + g*out[i-d]` |

`d` (samples) and `g` (gain) form a two-element vector. Default gain if omitted: 0.4.

```
W: w 100 0.9 y R     / comb filter on noise, resonance at ~441 Hz
```

### additive synthesis

| Verb | Usage | Description |
|------|-------|-------------|
| `o` | `P o H` | sum sin(P×h) for each h in H, equal amplitudes |
| `$` | `P $ A` | sum A[j]×sin(P×(j+1)) — weighted harmonic series |

`P` must be a phase ramp (from `+\`). Index 0 of A = harmonic 1. Use 0.0 for absent harmonics.

```
H: 1 3 5 7
W: w P o H              / odd harmonics, equal amplitude

A: 1 0.5 0.333 0.25
W: w P $ A              / sawtooth approximation

/ organ drawbar: h1,2,3,4 equal, h5 silent, h6 on, h7 silent, h8 on
A: 1 1 1 1 0 1 0 1
W: w P $ A
```

### wavetable oscillator

| Verb | Usage | Description |
|------|-------|-------------|
| `t` | `T t freq dur` | DDS oscillator from table T, freq Hz, dur samples |

`freq` and `dur` form a two-element vector. Linear interpolation between table samples. Monadic `t` is tan.

```
N: 1024
P: ~N                   / phase ramp 0..2π
T: s P                  / sine wavetable
D: 88200
W: w T t 440 D          / 440 Hz for 2 seconds
```

### pitched buzz (dyadic form)

| Verb | Usage | Description |
|------|-------|-------------|
| `b` | `freq b V` | 6-oscillator buzz at freq Hz, length = len(V) |

```
W: w e(T*(0-5%N)) * 220 b T   / decaying buzz at 220 Hz
```

### anti-click ramp (dyadic form)

| Verb | Usage | Description |
|------|-------|-------------|
| `u` | `N u V` | ramp 0→1 over N samples, then 1.0 |

```
W: w (100 u T) * s P    / 100-sample onset ramp on a sine
```

### quantize (dyadic form)

| Verb | Usage | Description |
|------|-------|-------------|
| `v` | `N v signal` | quantize to N levels |

```
Q: 8 v s P              / 8-level quantized sine
```

### stereo

| Verb | Usage | Description |
|------|-------|-------------|
| `z` | `L z R` | interleave into stereo stream `[l0,r0,l1,r1,...]` |

Output length is `min(L,R) * 2`. Extract channels with monadic `j` (left) and `k` (right).

---

## scan adverb (`\`)

`op\V` — running accumulation over V, same length as V.

| Scan | Description |
|------|-------------|
| `+\V` | running sum — the phase accumulator primitive |
| `*\V` | running product |
| `-\V` | running subtraction |
| `%\V` | running division |
| `&\V` | running minimum |
| `|\V` | running maximum |
| `^\V` | running power |

`/` is not reduce/over — it starts a comment. Reduction is verb-specific: `+V` sums, `>V` peaks.

---

## functions

`{ expr }` defines a function. `x` = first argument, `y` = second.

```
F: { x+1 }      / define
F 3              / call: x=3 → 4

G: { x+y }
2 G 3            / call: x=2, y=3 → 5
```

The phase accumulator as a reusable function:

```
C: p2%p0
X: { +\(x#(y*C)) }   / x=length, y=freq
P: N X 440
Q: N X 660
W: w (s P)+(s Q)
```

---

## special syntax

`;` separates expressions on one line — only the last value is used:

```
A: 1; B: 2; A+B     / returns 3
```

`p0` = 44100 (sample rate). `pN` = N×π. `p2%p0` = 2π/44100 — the 1 Hz phase increment.

---

## right-associativity gotchas

```
/ linear mix — parens required
W: (S*.3)+(U*.7)    / correct
W: S*.3+U*.7        / wrong: S*(0.3+U*0.7)

/ FM hazard — s consumes its full right expression
s P + s Q           / parses as sin(P + sin(Q)) — FM, not a mix
(s P) + (s Q)       / correct mix of two sines

/ hard clip — & and | are right-associative
(S & 0.5) | -0.5    / correct clip to [-0.5, 0.5]
S & -0.5 | 0.5      / wrong: S & (-0.5 | 0.5) = S & 0.5
```

---

## standard patterns

```
/ oscillator
C: p2%p0
N: 44100
T: !N
P: +\(N#(440*C))
W: w s P

/ exponential decay envelope — k=6.9 → -60dB at end
E: e(T*(0-6.9%N))

/ pitch-sweeping oscillator (141Hz → 50Hz)
F: 50+91*e(T*(0-60%N))
P: +\(N#(F*(p2%p0)))

/ percussive rise-then-fall — peaks at sample N/k
X: T*e(T*(0-8%N))

/ FM bell: fast index decay, slow amplitude decay
A: e(T*(0-3%N))
I: 5*e(T*(0-12%N))
P: +\(N#(440*C))
W: w A*(s(P+I*s(P)))

/ filtered noise
R: r T
W: w 0.1 f R           / lowpass
W: w R-(0.1 f R)        / highpass
W: w (0.4 f R)-(0.05 f R)  / bandpass

/ wavetable sine, 2 seconds
P: ~1024
D: 88200
W: w (s P) t 440 D
```

---

## test suite

```sh
gcc -O2 test_ksynth.c ksynth.c -lm -o test_ksynth && ./test_ksynth
```

158 tests, 0 failures. Coverage: scalars, vectors, arithmetic, scan, normalize, sine/cosine, exp/log, filter cutoff, highpass, delay/comb, additive synthesis `o` and `$`, wavetable `t`, dot literals, right-assoc correctness, envelope decay, noise length, pitch sweep, rise-decay envelope, 1-bit noise, host array helpers.

See `api.md` for the C embedding API.
