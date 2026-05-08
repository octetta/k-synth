# k/synth technical specification

---

## 1. language model

ksynth is a right-associative array DSL. Every value is a `double[]` vector. Scalars are 1-element vectors. There are no loops, no conditionals, no mutable state between lines — only vector expressions evaluated top to bottom.

**Right-associativity:** `a op b op c` evaluates as `a op (b op c)`. Use explicit parentheses to override.

**Variables:** single uppercase letter `A`–`Z`. No multi-letter names.

**Output:** `W` is the conventional output variable. Every script must assign it via `W: w expr`. The `w` verb normalizes to peak ±1.0.

**Constants:** `p0` = 44100 (sample rate). `pN` = N×π for integer N≥1. `p2%p0` = 2π/44100, the per-sample radian increment for 1 Hz.

---

## 2. evaluation model

Each `ks_eval` call receives a null-terminated string of one or more newline-separated expressions. Lines are evaluated top to bottom. Variables assigned in earlier lines are visible in later lines within the same call, and persist across calls until cleared.

The evaluator is sandboxed:

- **Arena allocator:** all temporaries draw from a pre-allocated block (default 8 MB). The arena resets after each `ks_eval` call. Persistent variables (`A`–`Z`) are separately malloc'd and survive resets.
- **Gas limit:** total operation count per eval is bounded. Prevents runaway scripts.
- **Signal catching:** `SIGSEGV`, `SIGFPE`, `SIGILL` are caught via `sigsetjmp`/`siglongjmp` and reported as error codes. `feclearexcept` prevents FPE re-triggering on x86.

After any error, the arena resets and persistent variables written before the fault are preserved.

---

## 3. verb inventory

### monadic (prefix, one argument)

| Verb | Behavior |
|------|----------|
| `!N` | iota: `[0..N-1]` |
| `~N` | phase ramp: `[0, 2π/N, ..., 2π*(N-1)/N]` |
| `s` | sin elementwise |
| `c` | cos elementwise |
| `t` | tan elementwise |
| `h` | tanh(V) |
| `d` | tanh(3V) — harder clip |
| `a` | abs elementwise |
| `q` | sqrt(abs(V)) elementwise |
| `l` | log(abs(V)+ε) elementwise |
| `e` | exp(V), input clamped [-100,100] |
| `x` | exp(-5V) — fast decay shape |
| `_` | floor elementwise |
| `p` | 44100 if V=0, else π×V |
| `n` | MIDI note to Hz: `440×2^((V-69)/12)` |
| `i` | reverse vector |
| `j` | extract even samples (left channel from interleaved stereo) |
| `k` | extract odd samples (right channel from interleaved stereo) |
| `r` | white noise [-1,1], one sample per element |
| `m` | 1-bit metallic noise ±0.7, deterministic |
| `b` | band-limited buzz at 110 Hz (6-oscillator cluster) |
| `u` | anti-click ramp: 0→1 over first 10 samples, then 1.0 |
| `v` | quantize to 4 levels (nearest 0.25) |
| `w` | peak-normalize to ±1.0 |
| `+V` | sum all elements → scalar |
| `>V` | peak absolute value → scalar |

### dyadic (infix, two arguments)

| Op | Behavior |
|----|----------|
| `+` `-` `*` | add, subtract, multiply |
| `%` | divide (`%` is division, not modulo) |
| `^` | power: `abs(A)^B` |
| `&` `\|` | min, max elementwise |
| `<` `>` `=` | compare → 0.0 or 1.0 |
| `#` | tile: `N#V` repeats V cyclically to length N |
| `,` | concatenate |
| `f` | 2-pole lowpass: `ct f signal` or `ct rs f signal` |
| `g` | 2-pole lowpass in Hz: `hz g signal` or `hz q g signal` |
| `y` | feedback delay: `d g y signal` |
| `o` | additive synthesis, equal amplitude: `P o H` |
| `$` | additive synthesis, weighted: `P $ A` |
| `t` | wavetable DDS oscillator: `T t freq dur` |
| `z` | stereo interleave: `L z R` → `[l0,r0,l1,r1,...]` |
| `b` | pitched buzz: `freq b V` |
| `u` | anti-click ramp: `N u V` |
| `v` | quantize to N levels: `N v signal` |
| `n` | (absorbed into monadic — no dyadic form) |

### scan adverb

`op\V` — running accumulation, same length as V. Supported ops: `+`, `*`, `-`, `%`, `&`, `|`, `^`.

`/` starts a comment and is never a reduce/over operator.

---

## 4. arithmetic semantics

- **Element-wise:** all dyadic arithmetic applies element-wise.
- **Length:** result length = max of left and right lengths. The shorter side cycles.
- **Division by zero:** returns 0.0.
- **NaN/Inf:** clamped to ±1e6 by `safe_val` in power, filter, and delay outputs.
- **`^` operator:** `abs(A)^B` — absolute value before exponentiation prevents complex results.

---

## 5. filter details

Both `f` and `g` implement a two-pole Chamberlin-derived state variable topology. The lowpass tap is the output.

**`f` — normalised coefficient**

`ct` is a per-sample coefficient 0.0–0.95 (clamped). Resonance `rs` 0–3.98 (clamped). Feedback is from the lowpass tap, producing a broad shelf lift rather than a sharp resonant peak — stable across the full resonance range.

Approximate mapping: `fc ≈ ct × 44100 / (2π)`.

**`g` — Hz input**

`f_coeff = 2 × sin(π × hz / 44100)`, clamped to 1.99. `damp = 1/Q` (default Q=0.5, damp=2.0). Accepts a modulation vector: if `hz` has the same length as `signal`, each sample uses its own cutoff.

---

## 6. wavetable oscillator

`T t freq dur` — plays table T as a DDS oscillator at `freq` Hz for `dur` samples with linear interpolation. Phase increment per sample = `freq × tbl_len / 44100`. `freq` and `dur` form a two-element vector absorbed from adjacent scalars or variables.

Monadic `t` is `tan`.

---

## 7. additive synthesis

**`P o H`** — output length = len(P). For each output sample i: `Σ_j sin(P[i] × H[j])`. Equal amplitude per harmonic.

**`P $ A`** — output length = len(P). For each output sample i: `Σ_j A[j] × sin(P[i] × (j+1))`. A[0] = harmonic 1, A[1] = harmonic 2, etc. Use 0.0 for absent harmonics.

Both are O(len(P) × len(H or A)) — gas-limited.

---

## 8. feedback delay

`d g y signal` — `d` and `g` are a two-element vector (delay in samples, gain). Default gain 0.4 if only one element provided. `out[i] = signal[i] + g × out[i-d]`. Output is the same length as signal. Values are passed through `safe_val` to prevent runaway feedback.

---

## 9. parser constraints

- **Variables:** `A`–`Z` only. `x` and `y` are reserved inside function bodies.
- **`!` and `~`:** argument capped at 1,000,000. Values over the cap: `!` raises `KS_ERR_INVALID_ARGS`; `~` returns an empty vector.
- **`#`:** tile count capped at 1,000,000. Raises `KS_ERR_INVALID_ARGS` above the cap.
- **Negation vs subtraction:** a spaced leading minus is negation (continues a vector literal). A flush minus on a value is subtraction. Use `0-X` to negate an expression unambiguously.
- **Comments:** `/` to end of line.
- **Semicolons:** `;` separates expressions on one line; only the last value is returned.

---

## 10. error codes

| Code | Meaning |
|------|---------|
| `KS_OK` | success |
| `KS_ERR_SYNTAX` | malformed expression |
| `KS_ERR_OOM` | arena exhausted |
| `KS_ERR_GAS` | gas limit reached |
| `KS_ERR_SIGSEGV` | segmentation fault caught |
| `KS_ERR_SIGFPE` | floating point exception caught |
| `KS_ERR_SIGILL` | illegal instruction caught |
| `KS_ERR_INVALID_ARGS` | bad argument value (e.g. `!1e9`) |
| `KS_ERR_INTERNAL` | unexpected internal error |

---

## 11. embedding

The public C API is in `ksynth.h`. Core functions:

| Function | Description |
|----------|-------------|
| `ks_create(mem, gas)` | allocate context; mem=0 → 8MB default |
| `ks_eval(ctx, code, len)` | evaluate; returns caller-owned K or NULL |
| `ks_destroy(ctx)` | free all resources |
| `ks_clear_vars(ctx)` | clear A–Z, keep context |
| `bind_scalar(ctx, name, val)` | set a named variable from host |
| `bind_array_f32/i32/f64(ctx, name, n, src)` | set array variable from host |
| `k_copy_to_f32/i32/f64(x, dst, max_n)` | copy result to host array |
| `k_free(ctx, x)` | free a returned K |

See `api.md` for full documentation.

---

## 12. thread safety

Each `ks_ctx` is not thread-safe. Multiple contexts in separate threads are safe — the signal handler uses a thread-local pointer (`KS_TLS`) to find the active context.
