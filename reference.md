# ksynth verb reference

All verbs operate on vectors of doubles. Every variable is a single uppercase letter `A`–`Z`. Right-associativity applies throughout — use parentheses to control evaluation order. Constants `p0`=44100 (sample rate), `pN`=N×π for N≥1.

---

## assignment and functions

| Syntax | Meaning |
|--------|---------|
| `A: expr` | Assign result of `expr` to variable `A` |
| `F: { expr }` | Define function `F`; `x` = first arg, `y` = second arg |
| `F arg` | Call `F` with one argument (`x`=arg) |
| `a F b` | Call `F` with two arguments (`x`=a, `y`=b) |

```
/ phase accumulator as reusable function
C: p2%p0
X: { +\(x#(y*C)) }   / x=length, y=freq_hz
P: N X 440            / phase ramp for 440 Hz over N samples
```

---

## monadic verbs (one argument)

### iota and ramps

| Verb | Input | Output | Description |
|------|-------|--------|-------------|
| `!N` | scalar N | [0,1,...,N-1] | Integer index ramp |
| `~N` | scalar N | [0, 2π/N, ..., 2π*(N-1)/N] | Phase ramp 0 to ~2π |

```
T: !44100             / time index 0..44099
P: ~1024              / phase ramp for one cycle, 1024 samples
```

`~N` is shorthand for `+\(N#(p2%N))` — use it when building wavetables.

### reductions (return scalar)

| Verb | Description |
|------|-------------|
| `+V` | Sum all elements |
| `>V` | Peak absolute value (max of abs) |

```
S: s ~44100
E: +S                 / sum of all samples
P: >S                 / peak amplitude (should be ~1.0)
```

### output

| Verb | Description |
|------|-------------|
| `w V` | Peak-normalize `V` to ±1.0 and write as output |

`w` is always used to produce `W`. It normalizes so the peak is exactly 1.0.

```
W: w s ~44100
```

### math

| Verb | Description |
|------|-------------|
| `s V` | sin(V) element-wise |
| `c V` | cos(V) element-wise |
| `t V` | tan(V) element-wise |
| `h V` | tanh(V) — soft saturation |
| `d V` | tanh(3V) — soft clip, more aggressive than `h` |
| `a V` | abs(V) element-wise |
| `q V` | sqrt(abs(V)) element-wise |
| `l V` | log(abs(V)+ε) element-wise (natural log) |
| `e V` | exp(V), clamped to [-100,100] input range |
| `x V` | exp(-5V) — fast exponential decay shape |
| `_ V` | floor(V) element-wise |
| `p V` | `p0`=44100 if V=0, else π×V element-wise |

```
A: e(T*(0-3%N))       / exponential decay envelope
D: d 2*s P            / driven sine into soft clip
F: p 2                / 2π (= 6.28318...)
```

### noise and special waveforms

| Verb | Description |
|------|-------------|
| `r V` | White noise: uniform random [-1,1], one per element of V |
| `m V` | 1-bit metallic noise: deterministic ±0.7 pattern, good for cymbals |
| `b V` | Band-limited buzz: sum of 6 phase-shifted square waves, organ-like |
| `u V` | Ramp up: 0→1 over first 10 samples then stays at 1.0 (anti-click) |

```
R: r T                / white noise, N samples
C: m T                / metallic noise (hi-hat character)
O: b T                / organ buzz
```

`r` ignores the values in V and only uses its length. `m` is deterministic — same input gives same output, useful for reproducible percussion.

### MIDI and pitch

| Verb | Description |
|------|-------------|
| `n V` | MIDI note number to Hz: `440 * 2^((V-69)/12)` |

```
M: n69                / 440.0 Hz (concert A)
M: n60                / 261.63 Hz (middle C)
M: n +\(4#2)          / chord: four notes stepped by 2 semitones
```

### reverse and stereo extraction

| Verb | Description |
|------|-------------|
| `i V` | Reverse vector V |
| `j V` | Extract left channel from interleaved stereo (even samples) |
| `k V` | Extract right channel from interleaved stereo (odd samples) |

```
B: i A                / reverse of A
L: j W                / left channel of stereo W
R: k W                / right channel of stereo W
```

### quantize (monadic form)

| Verb | Description |
|------|-------------|
| `v V` | Quantize to 4 levels (floor to nearest 0.25) |

```
Q: v s P              / 4-level quantized sine — lo-fi effect
```

---

## scan adverb (`\`)

`op\V` applies `op` as a running accumulation over V. Produces a vector the same length as V.

| Scan | Description |
|------|-------------|
| `+\V` | Running sum (cumulative sum) |
| `*\V` | Running product |
| `-\V` | Running subtraction |
| `%\V` | Running division |
| `&\V` | Running minimum |
| `|\V` | Running maximum |

```
/ phase accumulator — the core oscillator primitive
P: +\(N#(440*C))      / running sum of N copies of phase increment

/ cumulative maximum — envelope follower shape
E: |\A
```

`+\` is by far the most used scan — it turns a constant phase increment into a phase ramp, which is the standard oscillator pattern in ksynth.

---

## dyadic verbs (two arguments)

### arithmetic (element-wise, length = max of inputs, shorter cycles)

| Verb | Description |
|------|-------------|
| `A + B` | Addition |
| `A - B` | Subtraction |
| `A * B` | Multiplication |
| `A % B` | Division (not modulo — ksynth uses `%` for divide) |
| `A ^ B` | Power: `abs(A)^B` |
| `A & B` | Min element-wise (also: hard clip lower bound) |
| `A \| B` | Max element-wise (also: hard clip upper bound) |
| `A < B` | 1.0 if A<B else 0.0 |
| `A > B` | 1.0 if A>B else 0.0 |
| `A = B` | 1.0 if A=B else 0.0 |

```
/ hard clip to [-0.5, 0.5]
C: S & -0.5 | 0.5

/ silence second half of buffer
G: S * (T < (N%2))
```

Note: `%` is **division**, not modulo. To get the fractional part of X: `X - _(X)`.

### vector construction

| Verb | Description |
|------|-------------|
| `N # V` | Tile V to length N (repeat V cyclically) |
| `A , B` | Concatenate A and B |

```
/ 44100-sample constant at 440 Hz phase increment
F: 44100#(440*C)

/ drum pattern: kick, kick, snare, kick
Z: K,K,S,K
```

### filters

| Verb | Usage | Description |
|------|-------|-------------|
| `f` | `ct f signal` | Two-pole lowpass, `ct`=normalised coefficient 0–0.95 |
| `f` | `ct rs f signal` | Two-pole lowpass with resonance `rs` (0–3.9) |
| `g` | `hz g signal` | Two-pole lowpass, cutoff in Hz |
| `g` | `hz q g signal` | Two-pole lowpass in Hz with Q (0.01–3.9) |

```
L: 0.1 f R            / lowpass ~700 Hz
H: R - L              / highpass (zero resonance only)
B: (0.4 f R)-(0.05 f R) / bandpass

L: 800 g R            / same filter, cutoff in Hz
```

See [readme](readme.html) for resonance character notes.

### feedback delay

| Verb | Usage | Description |
|------|-------|-------------|
| `y` | `d g y signal` | Feedback delay: `out[i] = signal[i] + g*out[i-d]` |

`d` and `g` are passed as a two-element vector. Output is the same length as signal.

```
W: w 100 0.9 y R      / comb filter on noise, resonance at ~441 Hz
W: w 200 0.98 y R     / comb at ~220 Hz, longer sustain
```

### additive synthesis

| Verb | Usage | Description |
|------|-------|-------------|
| `o` | `P o H` | Sum sin(P×h) for each harmonic h in H, equal amplitudes |
| `$` | `P $ A` | Sum A[j]×sin(P×(j+1)) — weighted harmonic series |

```
H: 1 3 5 7
W: w P o H            / odd harmonics = hollow square-ish tone

A: 1 0.5 0.333 0.25
W: w P $ A            / weighted harmonics = sawtooth approximation
```

`P` should be a phase ramp (from `+\`). `o` and `$` are the main tools for additive synthesis.

### wavetable oscillator

| Verb | Usage | Description |
|------|-------|-------------|
| `t` | `T t freq dur` | Play table T as DDS oscillator at freq Hz for dur samples |

`freq` and `dur` form a two-element vector. Scalar variables absorb into the vector naturally: `T t 440 D` where D=88200 gives a two-second output.

```
N: 1024
P: ~N                 / one-cycle phase ramp
T: s P                / sine wavetable
D: 88200
W: w T t 440 D        / 440 Hz sine from table, 2 seconds

/ any waveform works as a table
T: (2*(P<p1%p2))-1    / square wave table (P < π)
W: w T t 220 D
```

Monadic `t` is `tan`.

### quantize (dyadic form)

| Verb | Usage | Description |
|------|-------|-------------|
| `v` | `N v signal` | Quantize signal to N levels |

```
Q: 8 v s P            / 8-level quantized sine — bit-crush effect
```

### stereo

| Verb | Usage | Description |
|------|-------|-------------|
| `z` | `L z R` | Interleave L and R into stereo stream [l0,r0,l1,r1,...] |

Output length is `min(L,R) * 2`. Use `j` and `k` to extract channels.

```
W: w L z R            / stereo output
```

---

## special syntax

### vectors literals

Space-separated numbers form a vector. A scalar variable following a number (with space) is absorbed into the vector:

```
V: 1 2 3 4            / four-element vector
V: 440 D              / [440, value-of-D] if D is scalar
```

### negative numbers in vectors

A minus sign with preceding space is negation (continues the vector). Flush minus is subtraction (ends the vector):

```
V: 1 -2 3             / [1, -2, 3]
V: A-1                / A minus 1 (subtraction)
```

### comments

```
/ this is a comment — everything after / to end of line
```

### semicolons

`;` separates expressions on one line; only the last value is used:

```
A: 1; B: 2; A+B       / evaluates all three, returns 3
```

---

## quick patterns

```
/ oscillator
C: p2%p0              / 2π/44100 — per-sample increment for 1 Hz
P: +\(N#(440*C))      / 440 Hz phase ramp over N samples
W: w s P              / sine wave output

/ envelope + oscillator
T: !N
A: e(T*(0-3%N))       / exponential decay
W: w A*s P            / enveloped sine

/ reusable oscillator function
X: { +\(x#(y*C)) }    / x=length, y=freq
P: N X 440
Q: N X 660
W: w (s P)+(s Q)      / two-voice chord

/ filtered noise
R: r T
W: w 0.05 f R         / lowpass filtered white noise

/ FM synthesis
I: 3.5*e(T*(0-40%N))  / fast-decaying modulation index
W: w A*(s P+(I*s P))  / self-FM bell tone
```
