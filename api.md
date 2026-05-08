# ksynth C API reference

ksynth is an embeddable DSP scripting engine. The host creates a context, evaluates ksynth expressions as strings, and reads results back as C arrays. All state lives in a `ks_ctx`; multiple contexts can coexist in the same process.

---

## types

```c
typedef struct { int r, n; double f[]; } *K;
```

`K` is a pointer to a vector of doubles. `n` is the element count, `f[]` is the data. A scalar is a `K` with `n=1`. A function literal is a `K` with `n=-1`; its `f[]` stores the body as a null-terminated string. The `r` field is a reference count — do not manipulate it directly.

```c
typedef enum {
    KS_OK = 0,
    KS_ERR_SYNTAX,        /* malformed ksynth code */
    KS_ERR_OOM,           /* arena exhausted */
    KS_ERR_GAS,           /* gas limit reached */
    KS_ERR_SIGSEGV,       /* caught segmentation fault */
    KS_ERR_SIGFPE,        /* caught floating point exception */
    KS_ERR_SIGILL,        /* caught illegal instruction */
    KS_ERR_INVALID_ARGS,  /* bad argument to verb (e.g. !1e9) */
    KS_ERR_INTERNAL
} ks_status;
```

---

## context lifecycle

| Function | Description |
|----------|-------------|
| `ks_create(mem_limit, gas_limit)` | Create a context |
| `ks_destroy(ctx)` | Free all resources |
| `ks_clear_vars(ctx)` | Free all A–Z variables, keep context |
| `ks_strerror(status)` | Human-readable status string |

`mem_limit` is the arena size in bytes. Pass `0` for the default (8 MB), which handles a 2-second stereo output at 44100 Hz with room for several intermediate buffers. Each sample is 8 bytes; a 1-second mono buffer is ~353 KB.

`gas_limit` caps total operations per eval. Pass `0` for no limit. A value of `50,000,000` is generous for most patches — enough for several seconds of multi-voice synthesis.

```c
ks_ctx *ctx = ks_create(0, 0);           / defaults: 8MB arena, no gas limit
ks_ctx *ctx = ks_create(4*1024*1024, 10000000LL);  / 4MB, 10M ops max
```

After each `ks_eval` call, inspect `ctx->last_status`:

```c
ks_eval(ctx, code, strlen(code));
if (ctx->last_status != KS_OK)
    fprintf(stderr, "%s\n", ks_strerror(ctx->last_status));
```

---

## evaluation

| Function | Description |
|----------|-------------|
| `ks_eval(ctx, code, len)` | Evaluate ksynth code, return last value |

`code` is the source string; `len` is its byte length. Returns a caller-owned `K` that must be freed with `k_free`. Returns `NULL` on error or if the expression produces no value.

The returned `K` is a fresh malloc'd copy — it stays valid across subsequent `ks_eval` calls. Variables (A–Z) assigned during eval persist in the context and can be read back via `ctx->vars['X'-'A']`.

```c
/ single expression
K r = ks_eval(ctx, "1+1", 3);
/ r->n == 1, r->f[0] == 2.0
k_free(ctx, r);

/ multi-line patch — variables persist after the call
const char *patch =
    "N: 44100\n"
    "T: !N\n"
    "A: e(T*(0-3%N))\n"
    "P: +\\(N#(440*(p2%p0)))\n"
    "W: w A*s P\n";
k_free(ctx, ks_eval(ctx, patch, strlen(patch)));

/ read result back from the context directly
K wave = ctx->vars['W'-'A'];   / valid until ks_clear_vars or next W: assignment
```

Multiple `ks_eval` calls share the same variable state — each call extends the session:

```c
ks_eval(ctx, "N: 44100", 8);
ks_eval(ctx, "T: !N",    5);    / N is visible here
ks_eval(ctx, "W: w s +\\(N#(440*(p2%p0)))", ...);
```

---

## reading results

After eval, persistent variables live in `ctx->vars[]`:

| Access | Description |
|--------|-------------|
| `ctx->vars['A'-'A']` | Direct access to variable A (valid until next assignment or `ks_clear_vars`) |
| `k_get(ctx, name)` | Arena copy of a variable — valid for current eval only |

`ctx->vars[i]` is a raw pointer into the context's persistent storage. Treat it as read-only from the host side — write through `bind_scalar` or `bind_array_*` instead.

```c
K w = ctx->vars['W'-'A'];
if (w && w->n > 0) {
    for (int i = 0; i < w->n; i++)
        send_sample(w->f[i]);
}
```

---

## memory management

| Function | Description |
|----------|-------------|
| `k_free(ctx, x)` | Free a K returned by `ks_eval` or `k_from_*`; no-op for NULL or arena objects |
| `k_new(ctx, n)` | Arena-allocate a K of n doubles (eval lifetime only) |
| `k_new_perm(ctx, n)` | Persistent malloc'd K; free with `free()` directly |

`k_free` is safe to call on any `K` — it detects arena vs malloc'd objects automatically. Arena objects (allocated during eval) are reclaimed in bulk when the arena resets; `k_free` is a no-op for them. Malloc'd objects (returned by `ks_eval`, created by `k_new_perm`, or stored by `bind_*`) are refcount-decremented and freed when the count hits zero.

Always call `k_free` on the value returned by `ks_eval`, even if you only care about side effects:

```c
k_free(ctx, ks_eval(ctx, "A: s ~44100", ...));  / A stored, return value discarded
```

---

## binding host data

| Function | Description |
|----------|-------------|
| `bind_scalar(ctx, name, val)` | Store a double as a named variable |
| `bind_array_f32(ctx, name, n, src)` | Store a float array as a named variable |
| `bind_array_i32(ctx, name, n, src)` | Store an int array as a named variable |
| `bind_array_f64(ctx, name, n, src)` | Store a double array as a named variable |

`name` must be `'A'`–`'Z'`. Data is copied and converted to `double[]` internally. Variables persist across evals. `n` is clamped to 1,000,000.

```c
/ bind synthesis parameters from host
bind_scalar(ctx, 'N', 44100.0);
bind_scalar(ctx, 'F', 261.63);     / middle C
k_free(ctx, ks_eval(ctx, "W: w s +\\(N#(F*(p2%p0)))", ...));

/ bind a host audio buffer for processing
float samples[1024];
capture_audio(samples, 1024);
bind_array_f32(ctx, 'I', 1024, samples);
k_free(ctx, ks_eval(ctx, "W: w 0.1 f I", ...));  / lowpass filter it
```

---

## converting between K and host arrays

| Function | Description |
|----------|-------------|
| `k_from_f32(ctx, n, src)` | Arena K from float array |
| `k_from_i32(ctx, n, src)` | Arena K from int array |
| `k_from_f64(ctx, n, src)` | Arena K from double array |
| `k_view(ctx, n, ptr)` | Arena K copying n doubles from ptr |
| `k_copy_to_f32(x, dst, max_n)` | Copy K to float array; returns count |
| `k_copy_to_i32(x, dst, max_n)` | Copy K to int array; returns count |
| `k_copy_to_f64(x, dst, max_n)` | Copy K to double array; returns count |

`k_from_*` returns an arena-allocated K valid only for the current eval. Use `bind_array_*` if the data needs to persist. `k_copy_to_*` returns the number of elements copied — `min(x->n, max_n)`.

```c
/ copy synthesis output to a float buffer for the audio driver
K result = ks_eval(ctx, "W", 1);
float out[44100];
int n = k_copy_to_f32(result, out, 44100);
k_free(ctx, result);
audio_write(out, n);
```

---

## function support

| Function | Description |
|----------|-------------|
| `k_is_func(x)` | Non-zero if x is a ksynth function literal |
| `k_func_body(x)` | Null-terminated body string, or NULL if not a function |
| `k_call(ctx, fn, args, nargs)` | Call a function K with 1 or 2 arguments |

`k_call` sets `x` = `args[0]` and `y` = `args[1]` inside the function body. Returns an arena K.

```c
k_free(ctx, ks_eval(ctx, "F: {x*x}", ...));     / define F
K fn = ctx->vars['F'-'A'];
float v = 5.0f;
K arg = k_from_f32(ctx, 1, &v);
K call_args[1] = {arg};
K result = k_call(ctx, fn, call_args, 1);
/ result->f[0] == 25.0
```

---

## signal safety and error recovery

ksynth catches `SIGSEGV`, `SIGFPE`, and `SIGILL` during eval using `sigsetjmp`/`siglongjmp`. On recovery, `ctx->last_status` is set and `ks_eval` returns `NULL`. Signal handlers are installed per-eval and restored afterward — ksynth does not permanently alter the host's signal disposition.

FP exceptions are cleared before each eval and again inside the signal handler (`feclearexcept(FE_ALL_EXCEPT)`), preventing re-triggering of the caught exception when execution resumes.

The gas limit triggers `KS_ERR_GAS` and unwinds cleanly. OOM triggers `KS_ERR_OOM`. In all error cases the arena is reset and variables written before the fault are preserved.

---

## thread safety

Each `ks_ctx` is not thread-safe — do not share a context between threads without a mutex. Multiple contexts in separate threads are fine; the signal handler uses a thread-local pointer (`KS_TLS`) so concurrent evals on different threads do not interfere.

---

## simple REPL example

A minimal REPL that reads ksynth expressions one line at a time, evaluates them, and prints the result. Variables accumulate across lines for the duration of the session.

```c
#include "ksynth.h"
#include <stdio.h>
#include <string.h>

int main(void) {
    ks_ctx *ctx = ks_create(0, 0);
    char line[4096];

    printf("ksynth repl — ctrl-d to quit\n");

    while (printf("> "), fgets(line, sizeof(line), stdin)) {
        size_t len = strlen(line);
        / strip trailing newline
        if (len > 0 && line[len-1] == '\n') line[--len] = '\0';
        if (len == 0) continue;

        K result = ks_eval(ctx, line, len);

        if (ctx->last_status != KS_OK) {
            printf("error: %s\n", ks_strerror(ctx->last_status));
        } else if (result) {
            p(ctx, result);   / prints scalar, vector preview, or function body
        }

        k_free(ctx, result);
    }

    ks_destroy(ctx);
    return 0;
}
```

Sample session:

```
> 2+2
4
> A: 1 2 3 4
> A
[1 2 3 4]
> +A
10
> F: {x*x}
> F 7
49
> W: w s ~44100
> W
[0 0.000143 0.000285 0.000428 0.000571 0.000713 0.000856 0.000999 ... (44100 total)]
```

Variables accumulate across lines. Typing `A: expr` stores the result silently (the assignment returns a value but the REPL prints it). Clear state between sessions with `ks_clear_vars(ctx)`.

---

## complete synthesis example

```c
#include "ksynth.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main(void) {
    ks_ctx *ctx = ks_create(0, 50000000LL);

    / bind parameters
    bind_scalar(ctx, 'N', 44100.0);
    bind_scalar(ctx, 'F', 261.63);   / middle C

    / run patch
    const char *patch =
        "T: !N\n"
        "C: p2%p0\n"
        "P: +\\(N#(F*C))\n"
        "A: e(T*(0-5%N))\n"
        "W: w A*s P\n";

    k_free(ctx, ks_eval(ctx, patch, strlen(patch)));

    if (ctx->last_status != KS_OK) {
        fprintf(stderr, "error: %s\n", ks_strerror(ctx->last_status));
        ks_destroy(ctx);
        return 1;
    }

    / copy output to float buffer
    K wave = ctx->vars['W'-'A'];
    float *buf = malloc(wave->n * sizeof(float));
    int n = k_copy_to_f32(wave, buf, wave->n);

    printf("%d samples ready\n", n);
    / ... send buf to audio driver ...

    free(buf);
    ks_destroy(ctx);
    return 0;
}
```
