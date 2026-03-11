# ksynth Language Reference

A compact array-oriented DSL for procedural wavetable synthesis.
Single-character verbs operate on vectors of floats. Variables are single uppercase letters A–Z.

---

## Variables

Uppercase letters A–Z store vectors.

```
A: 1 2 3 4
B: A * 2
```

Assignment uses `:`. The right-hand side is evaluated and bound to the variable.

---

## Comments

`/` begins a line comment. Everything from `/` to end of line is ignored.

```
A: 1%H  / sawtooth amplitudes
W: w P o (H*A)  / normalize and synthesize
```

```
/ set up phase and harmonics
P: ~4096
H: !32
H: H+1
```

This follows K/Q convention. `/` has no other meaning in ksynth.

---

## Negation vs Subtraction

`-` follows K's rule of thumb:

- **Flush against a value on the left** → subtraction
- **No value to the left, or preceded by whitespace** → negation

```
9-4       → 5        (subtraction: value immediately left of -)
1 2 -3    → [1 2 -3] (negation: space before -, continues vector)
-3        → -3       (negation: start of expression)
A-1       → A minus 1
```

This means vector literals with negative elements work naturally:

```
F: 5 -3 2 -1
```

And subtraction is unambiguous as long as there is no space between the left operand and `-`:

```
H-8       → subtract 8 from each element of H
H - 8     → same (operator layer handles spaced case via dyadic -)
```

---



### Scalars

```
3.14
-1
0.5
```

### Vectors (space-separated)

```
F: 5 7 9 11 13 15 17 19
```

---

## Monadic Verbs

Monadic verbs take one argument to their right.

### `!` — Index generator

Produces integers 0 to N-1.

```
!8
```
```
0 1 2 3 4 5 6 7
```

Typical use — build harmonic series:

```
H: !32
H: H+1
```
```
1 2 3 ... 32
```

---

### `~` — Phase ramp

Produces N evenly-spaced values from 0 to 2π*(N-1)/N.

```
P: ~4096
```
```
0  0.001534  0.003068  ...  6.281651
```

Use as input to the `o` outer-product-sum for wavetable generation.

---

### `+` — Sum reduction

Returns the sum of all elements as a scalar.

```
+!4
```
```
6
```

---

### `>` — Peak absolute value

Returns the largest absolute value in the vector.

```
A: -3 1 2
>A
```
```
3
```

Useful for normalization: `A % >A`

---

### `<` `>` `=` — Comparison (dyadic)

Element-wise boolean comparisons. Return 1.0 (true) or 0.0 (false).

```
3 < 5    / 1
5 < 3    / 0
H < 8    / [1 1 1 1 1 1 1 0 0 ...]
H = 4    / [0 0 0 1 0 0 ...]
```

`>=` and `<=` are expressed using not-less / not-greater idioms:

```
1-(H<C)  / H >= C
1-(H>C)  / H <= C
```

This is the standard K idiom for these comparisons.

Monadic `>` (peak absolute value) is unchanged — only the dyadic form of `>` has comparison semantics.

---

### `w` — Peak normalize

Scales the vector so its peak absolute value is 1.0.

```
A: -3 1 2
wA
```
```
-1.0  0.333  0.667
```

---

### `s` — Sine

```
s P
```

Computes sin(x) element-wise.

---

### `c` — Cosine

```
c P
```

Computes cos(x) element-wise.

---

### `t` — Tangent

```
t P
```

---

### `h` — Hyperbolic tangent (soft clip)

```
h A
```

Computes tanh(x). Values saturate smoothly toward ±1.

---

### `a` — Absolute value

```
a A
```

---

### `q` — Square root

```
q A
```

Computes sqrt(|x|) element-wise.

---

### `e` — Exponential

```
e A
```

Computes exp(x), clamped to ±100 to prevent overflow.

Because `e` also appears in the operator list, it is ambiguous when written inline between two expressions. Always assign the result to a variable first:

```
E: e(0-D*D*.2)
A: (1%H)*E
```

Not:

```
A: (1%H)*e(0-D*D*.2)   /* e parsed as dyadic here */
```

---

### `l` — Natural log

```
l A
```

Computes log(|x| + 1e-10). Guarded against log(0).

---

### `p` — Pi scaling

```
p A
```

Multiplies each element by π. Special case: `p 0` returns 44100 (sample rate).

```
p 2
```
```
6.28318...
```

---

### `d` — Drive / soft distortion

```
d A
```

Computes tanh(x * 3). Adds harmonic content.

---

### `i` — Reverse

```
A: 1 2 3 4
iA
```
```
4 3 2 1
```

---

### `_` — Floor

```
_ A
```

Rounds each element down to nearest integer.

---

### `r` — Random noise

```
r A
```

Replaces each element with a uniform random value in [-1, 1]. Input values are ignored.

---

### `v` — Quantize to N levels

Monadic: quantizes to 4 levels (default).

```
v A
```

Dyadic: `N v A` quantizes to N levels.

```
16 v W        /* 16-level quantize */
256 v W       /* 8-bit quantize */
4 v W         /* same as monadic v */
```

Produces lo-fi staircase waveforms. Higher N = finer steps.

---

### `n` — MIDI note to frequency

```
n A
```

Converts MIDI note numbers to Hz. A4 = 440 Hz = note 69.

```
n 69
```
```
440.0
```

---

### `x` — Exponential decay

```
x A
```

Computes exp(-5 * v). Maps 0→1, 1→0.007. Useful for decay envelopes.

---

### `m` — 1-bit noise

```
m A
```

Deterministic hash-based 1-bit noise. Returns ±0.7.

---

### `b` — Band-limited buzz

```
b A
```

Sum of 6 square waves at inharmonic ratios. Produces bright, buzzy waveforms.

---

### `u` — Linear ramp (first 10 samples)

```
u A
```

Returns i/10 for i < 10, then 1.0. Simple attack ramp.

---

### `j` — Extract left channel

```
jS
```

From an interleaved stereo vector, returns every even-indexed sample.

---

### `k` — Extract right channel

```
kS
```

From an interleaved stereo vector, returns every odd-indexed sample.

---

## Dyadic Verbs

Dyadic verbs take arguments on both sides.

### `+` `-` `*` `%` — Arithmetic

`%` is division.

```
A: 1%H
```

Shorter vector tiles to match the longer:

```
2 * 1 2 3
```
```
2 4 6
```

Division by zero returns 0.

---

### `^` — Power

```
2^8
```
```
256
```

---

### `&` — Minimum (elementwise)

```
3&5
```
```
3
```

---

### `|` — Maximum (elementwise)

```
3|5
```
```
5
```

---

### `<` `>` `=` — Comparison

Element-wise boolean. Returns 1.0 or 0.0.

```
H < 8      / 1 where harmonic < 8, else 0
H > 4      / 1 where harmonic > 4, else 0
H = 1      / 1 only at fundamental
```

`>=` and `<=` via complement:

```
C: 8
A: (H<C)+(1-(H<C))*(C%H)  / flat below C, 1/h rolloff above
```

---

### `o` — Equal-amplitude additive outer-product-sum

```
P o H
```

Where P is a phase vector [N] and H is a harmonic-number vector [M].

For each phase point, sums sin(phase * harmonic) across all harmonics with **equal amplitude**:

```
result[i] = Σ_j sin(P[i] * H[j])
```

Returns a wavetable of length N. Use when all harmonics contribute equally (e.g. organ drawbar where you control which harmonics are present via the H vector itself). For amplitude-weighted synthesis use `$` instead.

```
/ equal-amplitude sum of harmonics 1, 3, 5
P: ~4096
W: w P o (1 3 5)
```

---

### `$` — Weighted additive synthesis from amplitude spectrum

```
P $ A
```

Where P is a phase vector [N] and A is an **amplitude spectrum** [M].

`A[j]` is the amplitude of harmonic `j+1`. Harmonic numbers are implicit — index 0 = h1, index 1 = h2, etc.

```
result[i] = Σ_j A[j] * sin(P[i] * (j+1))
```

Returns a wavetable of length N. This is the primary verb for spectral synthesis. Use zeros for absent harmonics.

**Sawtooth** (1/h rolloff, all harmonics):
```
P: ~4096
H: !32
H: H+1
A: 1%H
W: w P $ A
```

**Square** (odd harmonics only):
```
P: ~4096
H: !32
H: H+1
M: a(s(H*1.5707963))
A: (1%H)*M
W: w P $ A
```

**Organ drawbar** (specific harmonics via sparse spectrum):
```
P: ~4096
/ h: 1 2 3 4 5 6 7 8 9 10 11 12 ...16
A: 1 1 1 1 0 1 0 1 0 1  0  1  0  0  0  1
W: w P $ A
```

**Why `$` not `o` for amplitude weighting?**

`o` computes `sin(P[i] * B[j])` — sine is nonlinear, so `sin(P * H*A) ≠ A * sin(P * H)`. Pushing amplitudes inside the sine does not weight the partials; it distorts their frequency. `$` keeps amplitude weighting outside the sine where it belongs.

The name follows K/Q convention where `$` means "cast" or "convert" — here, converting a frequency-domain spectrum into a time-domain wavetable.

---

### `f` — Two-pole lowpass filter

```
C f Signal
```

`C` is a cutoff coefficient (0..0.95). Optionally a 2-element vector `[cutoff resonance]`.

```
0.1 f W
```

---

### `y` — Feedback delay

```
N y Signal
```

Delays by N samples. Optional second element sets feedback gain (default 0.4).

```
100 y W              /* 100 samples, 0.4 gain */
100 0.7 y W          /* 100 samples, 0.7 gain */
441 0.5 y W          /* ~10ms at 44100Hz, 0.5 gain */
```

---

### `#` — Tile to length

```
8 # A
```

Repeats A cyclically to produce a vector of length 8.

---

### `,` — Concatenate

```
A , B
```

Joins two vectors end to end.

---

### `z` — Stereo interleave

```
L z R
```

Interleaves left and right channels: L[0] R[0] L[1] R[1] ...

---

## Scan Adverb `\`

Placed after a verb, `\` produces a running (cumulative) result.

| Expression | Meaning         |
|------------|-----------------|
| `+\ A`     | Running sum     |
| `*\ A`     | Running product |
| `-\ A`     | Running difference |
| `%\ A`     | Running division |
| `&\ A`     | Running minimum |
| `|\ A`     | Running maximum |
| `^\ A`     | Running power   |

```
A: 1 1 1 1
+\ A
```
```
1 2 3 4
```

---

## Functions

### Definition

```
F: {x+1}
```

Function bodies use `x` (first argument) and `y` (second argument).

### Application

```
F 3
```
```
4
```

Functions can be applied to vectors:

```
F: {x*x}
F !5
```
```
0 1 4 9 16
```

---

## Sequencing

Semicolon evaluates multiple expressions; the last result is returned.

```
A: !8 ; B: A+1 ; +B
```

---

## Synthesis Patterns

### Sawtooth

```
P: ~4096
H: !32
H: H+1
A: 1%H
W: w P $ A
```

### Square (odd harmonics only)

```
P: ~4096
H: !32
H: H+1
M: a(s(H*1.5707963))
A: (1%H)*M
W: w P $ A
```

### Triangle (1/h² odd-only rolloff)

```
P: ~4096
H: !32
H: H+1
M: a(s(H*1.5707963))
A: (1%(H*H))*M
W: w P $ A
```

### Vowel formant (peak at harmonic 8)

```
P: ~4096
H: !32
H: H+1
D: H-8
E: e(0-D*D*.2)
A: (1%H)*E
W: w P $ A
```

### PPG/DW-8000 spectral sweep (cutoff at harmonic C)

```
P: ~4096
H: !32
H: H+1
C: 8
A: (H<C)+(1-(H<C))*(C%H)
W: w P $ A
```

Varying C from 1 to 32 sweeps from dull to bright, producing a DW-8000/PPG-style wavetable bank when repeated across wave slots.

### Organ drawbar (sparse spectrum)

```
P: ~4096
/ h: 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16
A: 1 1 1 1 0 1 0 1 0 1  0  1  0  0  0  1
W: w P $ A
```

### Vowel bank (8 formant centers)

```
P: ~4096
H: !64
H: H+1
F: 5 7 9 11 13 15 17 19
D: H-F
E: e(0-D*D*.08)
A: (1%H)*E
W: w P $ A
```

`D: H-F` tiles F across H via modular indexing, producing a blended multi-formant amplitude envelope.

### Equal-amplitude harmonic sum (use `o`)

When all harmonics contribute equally — for example when you want to choose *which* harmonics are present but not weight them — `o` is more direct:

```
/ bell-like: harmonics 1, 3, 5, 7 equal amplitude
P: ~4096
W: w P o (1 3 5 7)
```

---

## Quick Verb Reference

| Verb | Monadic | Dyadic |
|------|---------|--------|
| `!`  | Index 0..N-1 | — |
| `~`  | Phase ramp 0..2π | — |
| `+`  | Sum reduce | Add |
| `-`  | — | Subtract |
| `*`  | — | Multiply |
| `%`  | — | Divide (0-safe) |
| `^`  | — | Power |
| `&`  | — | Min |
| `\|` | — | Max |
| `>`  | Peak abs | Greater-than (boolean) |
| `<`  | — | Less-than (boolean) |
| `=`  | — | Equal (boolean) |
| `o`  | — | Equal-amplitude additive (harmonics as H vector) |
| `$`  | — | Weighted additive synthesis (amplitudes as spectrum array) |
| `s`  | Sine | — |
| `c`  | Cosine | — |
| `t`  | Tangent | — |
| `h`  | Tanh | — |
| `a`  | Abs | — |
| `q`  | Sqrt | — |
| `e`  | Exp (clamped) | — |
| `l`  | Log (guarded) | — |
| `p`  | Pi scale (0→44100) | — |
| `d`  | Drive tanh*3 | — |
| `i`  | Reverse | — |
| `w`  | Peak normalize | — |
| `_`  | Floor | — |
| `r`  | Random [-1,1] | — |
| `v`  | Quantize (4 levels default) | Quantize to N levels |
| `n`  | MIDI→Hz | — |
| `x`  | Exp decay | — |
| `m`  | 1-bit noise | — |
| `b`  | Buzz (6 partials) | — |
| `u`  | Linear ramp | — |
| `f`  | — | Lowpass filter |
| `y`  | — | Feedback delay [samples] or [samples gain] |
| `z`  | — | Stereo interleave |
| `j`  | Left channel | — |
| `k`  | Right channel | — |
| `#`  | — | Tile to length |
| `,`  | — | Concatenate |
| `\`  | Scan adverb | — |
