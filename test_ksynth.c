#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "ksynth.h"

/* --- Test harness --- */

static int pass = 0;
static int fail = 0;

static void reset_vars(void) {
    extern K vars[26];
    for (int i = 0; i < 26; i++) {
        if (vars[i]) { k_free(vars[i]); vars[i] = NULL; }
    }
}

static K run(const char *src) {
    char buf[4096];
    strncpy(buf, src, sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';
    char *s = buf;
    return e(&s);
}

static void check_scalar(const char *label, const char *src, double expected, double tol) {
    K x = run(src);
    if (!x) {
        printf("FAIL [%s]: got NULL\n", label);
        fail++; return;
    }
    if (x->n != 1) {
        printf("FAIL [%s]: expected scalar, got n=%d\n", label, x->n);
        k_free(x); fail++; return;
    }
    double got = x->f[0];
    k_free(x);
    if (fabs(got - expected) <= tol) {
        printf("pass [%s]: %.6f\n", label, got);
        pass++;
    } else {
        printf("FAIL [%s]: expected %.6f got %.6f\n", label, expected, got);
        fail++;
    }
}

static void check_len(const char *label, const char *src, int expected_n) {
    K x = run(src);
    if (!x) {
        printf("FAIL [%s]: got NULL\n", label);
        fail++; return;
    }
    int got = x->n;
    k_free(x);
    if (got == expected_n) {
        printf("pass [%s]: len=%d\n", label, got);
        pass++;
    } else {
        printf("FAIL [%s]: expected len=%d got len=%d\n", label, expected_n, got);
        fail++;
    }
}

static void check_elem(const char *label, const char *src, int idx, double expected, double tol) {
    K x = run(src);
    if (!x) {
        printf("FAIL [%s]: got NULL\n", label);
        fail++; return;
    }
    if (idx >= x->n) {
        printf("FAIL [%s]: index %d out of range (n=%d)\n", label, idx, x->n);
        k_free(x); fail++; return;
    }
    double got = x->f[idx];
    k_free(x);
    if (fabs(got - expected) <= tol) {
        printf("pass [%s]: [%d]=%.6f\n", label, idx, got);
        pass++;
    } else {
        printf("FAIL [%s]: [%d] expected %.6f got %.6f\n", label, idx, expected, got);
        fail++;
    }
}

static void check_range(const char *label, const char *src, double lo, double hi) {
    K x = run(src);
    if (!x) {
        printf("FAIL [%s]: got NULL\n", label);
        fail++; return;
    }
    int bad = 0;
    for (int i = 0; i < x->n; i++) {
        if (x->f[i] < lo || x->f[i] > hi) { bad = i + 1; break; }
    }
    k_free(x);
    if (!bad) {
        printf("pass [%s]: all in [%.3f, %.3f]\n", label, lo, hi);
        pass++;
    } else {
        printf("FAIL [%s]: element %d out of [%.3f, %.3f]\n", label, bad - 1, lo, hi);
        fail++;
    }
}

/* --- Tests --- */

static void test_index_gen(void) {
    printf("\n-- ! index generator --\n");
    check_len  ("!4 length",   "!4",    4);
    check_elem ("!4 first",    "!4",    0, 0.0, 1e-9);
    check_elem ("!4 last",     "!4",    3, 3.0, 1e-9);
    check_len  ("!1 length",   "!1",    1);
    check_len  ("!0 length",   "!0",    0);
}

static void test_phase_gen(void) {
    printf("\n-- ~ phase generator --\n");
    double twopi = 6.28318530717958647692;
    check_len  ("~8 length",   "~8",    8);
    check_elem ("~8 first=0",  "~8",    0, 0.0,            1e-9);
    check_elem ("~8 [1]",      "~8",    1, twopi * 1 / 8,  1e-9);
    check_elem ("~8 last",     "~8",    7, twopi * 7 / 8,  1e-9);
    /* last element must be strictly less than 2pi */
    check_range("~4096 range", "~4096", 0.0, twopi - 1e-9);
    check_len  ("~4096 length","~4096", 4096);
}

static void test_arithmetic(void) {
    printf("\n-- arithmetic --\n");
    check_scalar("2+3",    "2+3",   5.0,  1e-9);
    check_scalar("6%2",    "6%2",   3.0,  1e-9);
    check_scalar("3*4",    "3*4",   12.0, 1e-9);
    check_scalar("9-4",    "9-4",   5.0,  1e-9);   /* flush minus = subtraction */
    check_scalar("2^3",    "2^3",   8.0,  1e-9);
    check_scalar("div0",   "1%0",   0.0,  1e-9);
}

static void test_negation(void) {
    printf("\n-- negation vs subtraction --\n");
    /* flush minus = subtraction */
    check_scalar("9-4=5",      "9-4",       5.0,  1e-9);
    check_scalar("10-3=7",     "10-3",      7.0,  1e-9);
    /* spaced minus after value = negation, continues vector */
    check_len   ("1 -2 len=2", "1 -2",      2);
    check_elem  ("1 -2 [0]=1", "1 -2",      0,  1.0, 1e-9);
    check_elem  ("1 -2 [1]=-2","1 -2",      1, -2.0, 1e-9);
    /* leading minus = negation */
    check_scalar("-3 scalar",  "-3",        -3.0, 1e-9);
    /* vector with negatives */
    check_len   ("1 2 -3 len", "1 2 -3",    3);
    check_elem  ("1 2 -3 [2]", "1 2 -3",    2, -3.0, 1e-9);
    /* subtraction of variable */
    reset_vars();
    run("A: 10");
    check_scalar("A-3=7",      "A-3",       7.0,  1e-9);
}

static void test_vector_arithmetic(void) {
    printf("\n-- vector arithmetic --\n");
    /* H: !4; H+1 -> 1 2 3 4 */
    reset_vars();
    run("H:!4");
    check_elem("H[0]=0",    "H",     0, 0.0, 1e-9);
    check_elem("H+1 [0]",   "H+1",   0, 1.0, 1e-9);
    check_elem("H+1 [3]",   "H+1",   3, 4.0, 1e-9);
    /* broadcast: scalar op vector */
    check_elem("2*!4 [2]",  "2*!4",  2, 4.0, 1e-9);
}

static void test_sum_reduce(void) {
    printf("\n-- + sum reduction --\n");
    check_scalar("+!4",   "+!4",   6.0,  1e-9);  /* 0+1+2+3 */
    check_scalar("+!1",   "+!1",   0.0,  1e-9);
    check_scalar("+!0",   "+!0",   0.0,  1e-9);
}

static void test_peak_reduce(void) {
    printf("\n-- > peak absolute (monadic) --\n");
    reset_vars();
    run("A: -3 1 2");
    check_scalar(">A peak",   ">A",  3.0, 1e-9);
    check_scalar(">!1 peak",  ">!1", 0.0, 1e-9);
}

static void test_comparisons(void) {
    printf("\n-- < > = boolean comparisons (dyadic) --\n");
    /* scalars */
    check_scalar("3<5=1",    "3<5",    1.0, 1e-9);
    check_scalar("5<3=0",    "5<3",    0.0, 1e-9);
    check_scalar("3>5=0",    "3>5",    0.0, 1e-9);
    check_scalar("5>3=1",    "5>3",    1.0, 1e-9);
    check_scalar("3=3=1",    "3=3",    1.0, 1e-9);
    check_scalar("3=4=0",    "3=4",    0.0, 1e-9);
    /* vector vs scalar — element-wise */
    reset_vars();
    run("H: !5");
    run("H: H+1");   /* H = [1 2 3 4 5] */
    check_elem("H<3 [0]=1",  "H<3",  0, 1.0, 1e-9);  /* 1<3 */
    check_elem("H<3 [1]=1",  "H<3",  1, 1.0, 1e-9);  /* 2<3 */
    check_elem("H<3 [2]=0",  "H<3",  2, 0.0, 1e-9);  /* 3<3 */
    check_elem("H<3 [3]=0",  "H<3",  3, 0.0, 1e-9);  /* 4<3 */
    check_elem("H>3 [2]=0",  "H>3",  2, 0.0, 1e-9);  /* 3>3 */
    check_elem("H>3 [3]=1",  "H>3",  3, 1.0, 1e-9);  /* 4>3 */
    /* not-less idiom for >= : 1-(H<C) */
    check_elem("1-(H<3)[2]=1", "1-(H<3)", 2, 1.0, 1e-9);  /* 3>=3 */
    check_elem("1-(H<3)[1]=0", "1-(H<3)", 1, 0.0, 1e-9);  /* 2>=3 false */
    /* DW shelf: (H<C) + (1-(H<C))*(C%H) */
    run("C: 3");
    check_elem("shelf [0]",  "(H<C)+(1-(H<C))*(C%H)", 0, 1.0, 1e-6); /* h=1 < C, amp=1 */
    check_elem("shelf [2]",  "(H<C)+(1-(H<C))*(C%H)", 2, 1.0, 1e-6); /* h=3 = C, amp=1 */
    check_elem("shelf [3]",  "(H<C)+(1-(H<C))*(C%H)", 3, 0.75, 1e-6); /* h=4 > C, amp=C/h=3/4 */
}

static void test_normalize(void) {
    printf("\n-- w normalize --\n");
    reset_vars();
    run("A: -3 1 2");
    K x = run("wA");
    if (!x) { printf("FAIL [w NULL]\n"); fail++; return; }
    double peak = 0.0;
    for (int i = 0; i < x->n; i++) if (fabs(x->f[i]) > peak) peak = fabs(x->f[i]);
    k_free(x);
    if (fabs(peak - 1.0) < 1e-9) {
        printf("pass [w peak=1.0]\n"); pass++;
    } else {
        printf("FAIL [w peak=%.6f expected 1.0]\n", peak); fail++;
    }
    /* normalizing zero vector should not crash */
    reset_vars();
    run("Z: 0 0 0");
    K z = run("wZ");
    if (z) { k_free(z); printf("pass [w zero no crash]\n"); pass++; }
    else   { printf("FAIL [w zero returned NULL]\n"); fail++; }
}

static void test_sine(void) {
    printf("\n-- s sine --\n");
    check_scalar("s0",       "s 0",                    0.0,  1e-9);
    check_scalar("s pi/2",   "s 1.5707963267948966",   1.0,  1e-9);
    check_scalar("s pi",     "s 3.14159265358979",      0.0,  1e-6);
}

static void test_cosine(void) {
    printf("\n-- c cosine --\n");
    check_scalar("c0",      "c 0",                   1.0,  1e-9);
    check_scalar("c pi/2",  "c 1.5707963267948966",  0.0,  1e-9);
    check_scalar("c pi",    "c 3.14159265358979",    -1.0,  1e-6);
}

static void test_exp_log(void) {
    printf("\n-- e exp, l log --\n");
    check_scalar("e0=1",    "e 0",   1.0,       1e-9);
    check_scalar("e1=e",    "e 1",   2.718281828, 1e-6);
    check_scalar("e clamp", "e 200", exp(100),  1e-3);  /* clamped at 100 */
    check_scalar("l e=1",   "l 2.718281828", 1.0, 1e-6);
    check_scalar("l guard", "l 0",   log(1e-10), 1e-6); /* log(0) guarded */
}

static void test_reverse(void) {
    printf("\n-- i reverse --\n");
    reset_vars();
    run("A: !4");
    check_elem("iA [0]=3", "iA", 0, 3.0, 1e-9);
    check_elem("iA [3]=0", "iA", 3, 0.0, 1e-9);
    check_len ("iA len=4", "iA", 4);
}

static void test_scan(void) {
    printf("\n-- scan adverb --\n");
    /* +\ running sum of 1 1 1 1 -> 1 2 3 4 */
    reset_vars();
    run("A: 1 1 1 1");
    check_elem("+\\A [0]", "+\\A", 0, 1.0, 1e-9);
    check_elem("+\\A [3]", "+\\A", 3, 4.0, 1e-9);
    /* *\ running product of 1 2 3 4 -> 1 2 6 24 */
    reset_vars();
    run("A: 1 2 3 4");
    check_elem("*\\A [3]", "*\\A", 3, 24.0, 1e-9);
}

static void test_concat(void) {
    printf("\n-- , concat --\n");
    reset_vars();
    run("A: 1 2");
    run("B: 3 4");
    check_len ("A,B len=4",  "A,B", 4);
    check_elem("A,B [0]=1",  "A,B", 0, 1.0, 1e-9);
    check_elem("A,B [3]=4",  "A,B", 3, 4.0, 1e-9);
}

static void test_tile(void) {
    printf("\n-- # tile --\n");
    reset_vars();
    run("A: 1 2 3");
    check_len ("8#A len=8",  "8#A", 8);
    check_elem("8#A [0]=1",  "8#A", 0, 1.0, 1e-9);
    check_elem("8#A [3]=1",  "8#A", 3, 1.0, 1e-9);  /* wraps: 3%3=0 -> A[0]=1 */
    check_elem("8#A [4]=2",  "8#A", 4, 2.0, 1e-9);
}

static void test_minmax(void) {
    printf("\n-- & min, | max (dyadic) --\n");
    check_scalar("3&5=3",  "3&5", 3.0, 1e-9);
    check_scalar("3|5=5",  "3|5", 5.0, 1e-9);
    check_scalar("-1&1=-1","(-1)&1", -1.0, 1e-9);
}

static void test_additive_saw(void) {
    printf("\n-- additive sawtooth via o --\n");
    reset_vars();
    /* sawtooth: P o (H * 1/H) = P o 1 for all harmonics
     * with 1 harmonic this is just sin(phase) */
    run("P: ~4096");
    run("H: !8");
    run("H: H+1");
    run("A: 1%H");
    K w = run("P o (H*A)");
    if (!w) { printf("FAIL [o NULL]\n"); fail++; return; }
    int n = w->n;
    /* result must be 4096 samples */
    if (n != 4096) {
        printf("FAIL [o length=%d expected 4096]\n", n); fail++;
        k_free(w); return;
    }
    printf("pass [o length=4096]\n"); pass++;
    /* first sample: phase=0, sin(0*h)=0 for all h, sum=0 */
    if (fabs(w->f[0]) < 1e-9) {
        printf("pass [o [0]=0]\n"); pass++;
    } else {
        printf("FAIL [o [0]=%.6f expected 0]\n", w->f[0]); fail++;
    }
    /* all samples finite */
    int bad = 0;
    for (int i = 0; i < n; i++) if (isnan(w->f[i]) || isinf(w->f[i])) { bad++; }
    if (!bad) { printf("pass [o no NaN/Inf]\n"); pass++; }
    else      { printf("FAIL [o %d NaN/Inf samples]\n", bad); fail++; }
    k_free(w);
}

static void test_additive_square(void) {
    printf("\n-- additive square (odd harmonics) via o --\n");
    reset_vars();
    run("P: ~4096");
    run("H: !16");
    run("H: H+1");
    /* odd harmonics only: H%2 = 1 for odd, 0 for even */
    run("A: (1%H)*(H%2)");
    K w = run("P o (H*A)");
    if (!w) { printf("FAIL [square o NULL]\n"); fail++; return; }
    /* even harmonics contribute 0 amplitude, result should be nonzero */
    int nonzero = 0;
    for (int i = 1; i < w->n; i++) if (fabs(w->f[i]) > 1e-9) { nonzero = 1; break; }
    if (nonzero) { printf("pass [square nonzero]\n"); pass++; }
    else         { printf("FAIL [square all zero]\n"); fail++; }
    check_len("square len=4096", "P o (H*A)", 4096);
    k_free(w);
}

static void test_normalize_wavetable(void) {
    printf("\n-- normalize wavetable (w after o) --\n");
    reset_vars();
    run("P: ~4096");
    run("H: !8");
    run("H: H+1");
    run("A: 1%H");
    run("W: P o (H*A)");
    K n = run("wW");
    if (!n) { printf("FAIL [wW NULL]\n"); fail++; return; }
    double peak = 0.0;
    for (int i = 0; i < n->n; i++) if (fabs(n->f[i]) > peak) peak = fabs(n->f[i]);
    k_free(n);
    if (fabs(peak - 1.0) < 1e-6) {
        printf("pass [wW peak=1.0]\n"); pass++;
    } else {
        printf("FAIL [wW peak=%.6f]\n", peak); fail++;
    }
}

static void test_vowel_pattern(void) {
    printf("\n-- vowel pattern (formant via e) --\n");
    reset_vars();
    run("P: ~4096");
    run("H: !32");
    run("H: H+1");
    run("D: H-8");
    run("E: e(0-D*D*.2)");
    run("A: (1%H)*E");
    K w = run("P o (H*A)");
    if (!w) { printf("FAIL [vowel NULL]\n"); fail++; return; }
    check_len("vowel len=4096", "P o (H*A)", 4096);
    /* amplitudes should be shaped — element 8 (formant) should have
     * higher amplitude contribution than element 1 */
    K a = run("A");
    if (a && a->n >= 9) {
        double a1 = a->f[0];   /* harmonic 1 */
        double a8 = a->f[7];   /* harmonic 8 (formant center) */
        if (a8 > a1) { printf("pass [vowel formant peak at h=8]\n"); pass++; }
        else         { printf("FAIL [vowel a8=%.4f not > a1=%.4f]\n", a8, a1); fail++; }
        k_free(a);
    }
    k_free(w);
}

static void test_quantize(void) {
    printf("\n-- v quantize --\n");
    reset_vars();
    run("A: 0.9 0.5 0.1");
    /* monadic: 4 levels */
    check_elem("v mono [0]",  "vA",    0, 0.75, 1e-9);
    check_elem("v mono [2]",  "vA",    2, 0.0,  1e-9);
    /* dyadic: 2 levels */
    check_elem("2vA [0]",     "2 v A", 0, 0.5,  1e-9);
    /* dyadic: 16 levels */
    check_elem("16vA [0]",    "16 v A", 0, 0.875, 1e-9);
    /* dyadic: 256 levels (8-bit) */
    run("B: 0.5");
    check_elem("256vB",       "256 v B", 0, 0.5, 1e-9);
}

static void test_delay(void) {
    printf("\n-- y feedback delay --\n");
    reset_vars();
    /* with default gain 0.4: first N samples unaffected by feedback */
    run("S: 1 0 0 0 0 0 0 0");
    K d = run("4 y S");
    if (!d) { printf("FAIL [y NULL]\n"); fail++; return; }
    check_len("y len=8", "4 y S", 8);
    /* sample 0 = input 1, no feedback yet */
    if (fabs(d->f[0] - 1.0) < 1e-9) { printf("pass [y [0]=1.0]\n"); pass++; }
    else { printf("FAIL [y [0]=%.6f]\n", d->f[0]); fail++; }
    /* sample 4 = 0 + 0.4 * d[0] = 0.4 */
    if (fabs(d->f[4] - 0.4) < 1e-6) { printf("pass [y [4]=0.4]\n"); pass++; }
    else { printf("FAIL [y [4]=%.6f expected 0.4]\n", d->f[4]); fail++; }
    k_free(d);
    /* explicit gain 0.7 */
    reset_vars();
    run("S: 1 0 0 0 0 0 0 0");
    K d2 = run("4 0.7 y S");
    if (!d2) { printf("FAIL [y gain NULL]\n"); fail++; return; }
    if (fabs(d2->f[4] - 0.7) < 1e-6) { printf("pass [y gain [4]=0.7]\n"); pass++; }
    else { printf("FAIL [y gain [4]=%.6f expected 0.7]\n", d2->f[4]); fail++; }
    k_free(d2);
}

static void test_comments(void) {
    printf("\n-- / line comments --\n");
    reset_vars();
    /* comment on its own line */
    check_scalar("comment line",  "/ this is ignored\n3",    3.0, 1e-9);
    /* comment after expression */
    check_scalar("inline comment","3+1 / adds one",          4.0, 1e-9);
    /* assignment then comment */
    reset_vars();
    run("A: 42 / the answer");
    check_scalar("assign+comment","A",                        42.0, 1e-9);
    /* comment does not affect next line */
    reset_vars();
    run("/ ignore me");
    run("B: 7");
    check_scalar("after comment", "B",                        7.0, 1e-9);
}

static void test_function_literal(void) {
    printf("\n-- function literals --\n");
    reset_vars();
    /* F: {x+1} ; F 3 -> 4 */
    run("F: {x+1}");
    check_scalar("fn x+1",    "F 3",   4.0, 1e-9);
    check_scalar("fn x*2",    "F 9",   10.0, 1e-9);
    /* two-arg function */
    run("G: {x+y}");
    reset_vars();
    run("G: {x+y}");
    /* direct call via expression — dyadic application */
    K gfn = run("G");
    if (k_is_func(gfn)) {
        printf("pass [G is function]\n"); pass++;
        k_free(gfn);
    } else {
        printf("FAIL [G not function]\n"); fail++;
        if (gfn) k_free(gfn);
    }
}

static void test_semicolon(void) {
    printf("\n-- semicolon sequencing --\n");
    reset_vars();
    /* last result wins */
    check_scalar("1;2;3", "1;2;3", 3.0, 1e-9);
}

static void test_stereo(void) {
    printf("\n-- z stereo interleave, j/k extract --\n");
    reset_vars();
    run("L: 1 2 3 4");
    run("R: 5 6 7 8");
    run("S: L z R");
    check_len ("S len=8",   "S",  8);
    check_elem("S[0]=1",    "S",  0, 1.0, 1e-9);
    check_elem("S[1]=5",    "S",  1, 5.0, 1e-9);
    check_elem("S[6]=4",    "S",  6, 4.0, 1e-9);
    check_elem("S[7]=8",    "S",  7, 8.0, 1e-9);
    check_elem("jS[0]=1",   "jS", 0, 1.0, 1e-9);
    check_elem("kS[0]=5",   "kS", 0, 5.0, 1e-9);
}

static void test_safe_val(void) {
    printf("\n-- safe_val: NaN/Inf protection --\n");
    /* 0^0 in the codebase uses pow(fabs(a),b) — should not produce NaN */
    check_scalar("0^0", "0^0", 1.0, 1e-9);
    /* division by zero guarded */
    check_scalar("1%0", "1%0", 0.0, 1e-9);
}

static void test_pi_scaling(void) {
    printf("\n-- p pi scaling --\n");
    check_scalar("p 1 = pi",  "p 1", 3.14159265358979, 1e-9);
    check_scalar("p 2 = 2pi", "p 2", 6.28318530717958, 1e-9);
    check_scalar("p 0 = sr",  "p 0", 44100.0,          1e-9);
}

/* --- $ : weighted additive synthesis tests --- */

static void test_dollar_basic(void) {
    printf("\n-- $ weighted additive synthesis --\n");
    reset_vars();
    run("P: ~4096");

    /* single harmonic: P $ (1.0) = sin(P*1) = s P */
    K a = run("P $ 1.0");
    K b = run("s P");
    if (!a || !b) { printf("FAIL [$ single NULL]\n"); fail++; k_free(a); k_free(b); return; }
    if (a->n != 4096) { printf("FAIL [$ length=%d]\n", a->n); fail++; }
    else               { printf("pass [$ length=4096]\n"); pass++; }
    double maxdiff = 0.0;
    for (int i = 0; i < a->n; i++) {
        double d = fabs(a->f[i] - b->f[i]);
        if (d > maxdiff) maxdiff = d;
    }
    if (maxdiff < 1e-9) { printf("pass [$ h1 == s P]\n"); pass++; }
    else                 { printf("FAIL [$ h1 max diff=%.2e]\n", maxdiff); fail++; }
    k_free(a); k_free(b);
}

static void test_dollar_sawtooth(void) {
    printf("\n-- $ sawtooth spectrum --\n");
    reset_vars();
    run("P: ~4096");
    /* sawtooth: b[j] = 1/(j+1) for j=0..31 */
    run("H: !32");
    run("H: H+1");
    run("A: 1%H");
    K w = run("w P $ A");
    if (!w) { printf("FAIL [$ saw NULL]\n"); fail++; return; }
    if (w->n != 4096) { printf("FAIL [$ saw length=%d]\n", w->n); fail++; }
    else               { printf("pass [$ saw length=4096]\n"); pass++; }
    /* peak should be normalised to 1.0 */
    double pk = 0.0;
    for (int i = 0; i < w->n; i++) if (fabs(w->f[i]) > pk) pk = fabs(w->f[i]);
    if (fabs(pk - 1.0) < 1e-6) { printf("pass [$ saw peak=1.0]\n"); pass++; }
    else                         { printf("FAIL [$ saw peak=%.6f]\n", pk); fail++; }
    /* first sample: phase=0, sin(0)=0 for all harmonics */
    if (fabs(w->f[0]) < 1e-9) { printf("pass [$ saw [0]=0]\n"); pass++; }
    else                        { printf("FAIL [$ saw [0]=%.6f]\n", w->f[0]); fail++; }
    /* no NaN/Inf */
    int bad = 0;
    for (int i = 0; i < w->n; i++) if (isnan(w->f[i]) || isinf(w->f[i])) bad++;
    if (!bad) { printf("pass [$ saw no NaN/Inf]\n"); pass++; }
    else       { printf("FAIL [$ saw %d NaN/Inf]\n", bad); fail++; }
    k_free(w);
}

static void test_dollar_square(void) {
    printf("\n-- $ square: odd harmonics only --\n");
    reset_vars();
    run("P: ~4096");
    /* square: 1/h for odd h, 0 for even — encode as spectrum array */
    /* A[j]=0 for even j (h=2,4,6…), A[j]=1/(j+1) for odd j (h=1,3,5…) */
    run("H: !16");
    run("H: H+1");
    run("M: a(s(H*1.5707963))");
    run("A: (1%H)*M");
    K w = run("w P $ A");
    if (!w) { printf("FAIL [$ square NULL]\n"); fail++; return; }
    /* result must be nonzero */
    int nonzero = 0;
    for (int i = 1; i < w->n; i++) if (fabs(w->f[i]) > 1e-9) { nonzero = 1; break; }
    if (nonzero) { printf("pass [$ square nonzero]\n"); pass++; }
    else          { printf("FAIL [$ square all zero]\n"); fail++; }
    /* peak normalised */
    double pk = 0.0;
    for (int i = 0; i < w->n; i++) if (fabs(w->f[i]) > pk) pk = fabs(w->f[i]);
    if (fabs(pk - 1.0) < 1e-6) { printf("pass [$ square peak=1.0]\n"); pass++; }
    else                         { printf("FAIL [$ square peak=%.6f]\n", pk); fail++; }
    k_free(w);
}

static void test_dollar_organ(void) {
    printf("\n-- $ organ: sparse spectrum via zero padding --\n");
    reset_vars();
    run("P: ~4096");
    /* organ drawbar: h1,2,3,4 equal, h5=0, h6 on, h7=0, h8 on ... */
    /* encode as: A[0..15], non-zero at indices 0,1,2,3,5,7,9,11,15 */
    run("A: 1 1 1 1 0 1 0 1 0 1 0 1 0 0 0 1");
    K w = run("w P $ A");
    if (!w) { printf("FAIL [$ organ NULL]\n"); fail++; return; }
    if (w->n == 4096) { printf("pass [$ organ length=4096]\n"); pass++; }
    else               { printf("FAIL [$ organ length=%d]\n", w->n); fail++; }
    double pk = 0.0;
    for (int i = 0; i < w->n; i++) if (fabs(w->f[i]) > pk) pk = fabs(w->f[i]);
    if (fabs(pk - 1.0) < 1e-6) { printf("pass [$ organ peak=1.0]\n"); pass++; }
    else                         { printf("FAIL [$ organ peak=%.6f]\n", pk); fail++; }
    k_free(w);
}

static void test_dollar_amplitude_ordering(void) {
    printf("\n-- $ amplitude ordering: h1 louder than h8 in 1/h spectrum --\n");
    reset_vars();
    run("P: ~4096");
    run("H: !8");
    run("H: H+1");
    run("A: 1%H");
    /* extract per-harmonic contribution by using single-element spectra */
    /* h1 only: A1 = 1.0 in slot 0 */
    K w1 = run("P $ 1.0");
    /* h8 only: A8 = slot 7, so 7 zeros then 1/8 */
    run("Z: 0 0 0 0 0 0 0 0.125");
    K w8 = run("P $ Z");
    if (!w1 || !w8) { printf("FAIL [$ ordering NULL]\n"); fail++; k_free(w1); k_free(w8); return; }
    double pk1 = 0.0, pk8 = 0.0;
    for (int i = 0; i < w1->n; i++) if (fabs(w1->f[i]) > pk1) pk1 = fabs(w1->f[i]);
    for (int i = 0; i < w8->n; i++) if (fabs(w8->f[i]) > pk8) pk8 = fabs(w8->f[i]);
    if (pk1 > pk8) { printf("pass [$ h1 amp=%.4f > h8 amp=%.4f]\n", pk1, pk8); pass++; }
    else            { printf("FAIL [$ h1=%.4f h8=%.4f]\n", pk1, pk8); fail++; }
    k_free(w1); k_free(w8);
}

static void test_dollar_vs_o(void) {
    printf("\n-- $ vs o: equal-amplitude case should match --\n");
    reset_vars();
    run("P: ~4096");
    /* o with harmonic numbers 1,2,3 gives equal-amplitude sum */
    run("H: 1 2 3");
    K wo = run("P o H");
    /* $ with spectrum 1 1 1 (h1=1, h2=1, h3=1) should match */
    K wd = run("P $ 1 1 1");
    if (!wo || !wd) { printf("FAIL [$/o NULL]\n"); fail++; k_free(wo); k_free(wd); return; }
    double maxdiff = 0.0;
    for (int i = 0; i < wo->n && i < wd->n; i++) {
        double d = fabs(wo->f[i] - wd->f[i]);
        if (d > maxdiff) maxdiff = d;
    }
    if (maxdiff < 1e-9) { printf("pass [$ matches o for equal amps, maxdiff=%.2e]\n", maxdiff); pass++; }
    else                 { printf("FAIL [$/o maxdiff=%.2e]\n", maxdiff); fail++; }
    k_free(wo); k_free(wd);
}

static void test_dollar_formant(void) {
    printf("\n-- $ formant: Gaussian peak at h=9 (EE vowel) --\n");
    reset_vars();
    run("P: ~4096");
    run("H: !32");
    run("H: H+1");
    run("D: H-9");
    run("G: e(0-D*D*.2)");
    run("A: (1%H)*G");
    K w = run("w P $ A");
    if (!w) { printf("FAIL [$ formant NULL]\n"); fail++; return; }
    /* check A has peak near h=9 (index 8) */
    K a = run("A");
    if (a && a->n >= 16) {
        double a9 = a->f[8];   /* h=9 */
        double a1 = a->f[0];   /* h=1 */
        double a16 = a->f[15]; /* h=16 */
        if (a9 > a1 && a9 > a16) { printf("pass [$ formant peak at h=9]\n"); pass++; }
        else { printf("FAIL [$ formant a1=%.4f a9=%.4f a16=%.4f]\n", a1, a9, a16); fail++; }
        k_free(a);
    }
    if (w->n == 4096) { printf("pass [$ formant length=4096]\n"); pass++; }
    else               { printf("FAIL [$ formant length=%d]\n", w->n); fail++; }
    k_free(w);
}

/* --- Main --- */

int main(void) {
    printf("ksynth test suite\n");
    printf("=================\n");

    test_index_gen();
    test_phase_gen();
    test_arithmetic();
    test_negation();
    test_vector_arithmetic();
    test_sum_reduce();
    test_peak_reduce();
    test_comparisons();
    test_normalize();
    test_sine();
    test_cosine();
    test_exp_log();
    test_reverse();
    test_scan();
    test_concat();
    test_tile();
    test_minmax();
    test_additive_saw();
    test_additive_square();
    test_normalize_wavetable();
    test_vowel_pattern();
    test_quantize();
    test_delay();
    test_comments();
    test_function_literal();
    test_semicolon();
    test_stereo();
    test_safe_val();
    test_pi_scaling();
    test_dollar_basic();
    test_dollar_sawtooth();
    test_dollar_square();
    test_dollar_organ();
    test_dollar_amplitude_ordering();
    test_dollar_vs_o();
    test_dollar_formant();

    printf("\n=================\n");
    printf("passed: %d  failed: %d\n", pass, fail);
    return fail > 0 ? 1 : 0;
}
