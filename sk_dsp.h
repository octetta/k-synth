#ifndef SK_DSP_H
#define SK_DSP_H

#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#ifndef M_PI_2
#define M_PI_2 1.57079632679489661923
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* =========================================================================
 * 1. Numerical Safety & Clamping
 * ========================================================================= */

static inline float sk_clampf(float val, float min_val, float max_val) {
    if (isnan(val)) return min_val;
    return fmaxf(min_val, fminf(val, max_val));
}

static inline double sk_clampd(double val, double min_val, double max_val) {
    if (isnan(val)) return min_val;
    return fmax(min_val, fmin(val, max_val));
}

static inline int sk_clampi(int val, int min_val, int max_val) {
    return val < min_val ? min_val : (val > max_val ? max_val : val);
}

static inline double sk_safe_val(double v) {
    if (isnan(v) || isinf(v)) return 0.0;
    if (v > 1e6) return 1e6;
    if (v < -1e6) return -1e6;
    return v;
}

static inline float sk_denormal_flush(float v) {
    return (fabsf(v) < 1e-15f) ? 0.0f : v;
}

/* =========================================================================
 * 2. Pitch, Decibel, Panning & Fast Transcendental Math
 * ========================================================================= */

/* Fast power approximation using IEEE 754 bit manipulation (~10x faster than powf) */
static inline float sk_fast_pow(float a, float b) {
    if (a <= 0.0f) return 0.0f;
    union { float f; int32_t i; } u;
    u.f = a;
    u.i = (int32_t)(b * (float)(u.i - 1065353216) + 1065353216.0f);
    return u.f;
}

static inline float sk_mtof(float midi_note) {
    return 440.0f * powf(2.0f, (midi_note - 69.0f) / 12.0f);
}

static inline float sk_ftom(float freq_hz) {
    if (freq_hz <= 0.0f) return 0.0f;
    return 69.0f + 12.0f * log2f(freq_hz / 440.0f);
}

/* Decibel <-> Linear amplitude conversions */
static inline float sk_db_to_linear(float db) {
    return powf(10.0f, db * 0.05f);
}

static inline float sk_linear_to_db(float linear) {
    return linear > 0.00001f ? 20.0f * log10f(linear) : -100.0f;
}

/* Constant-power stereo panning law (equal acoustic power: L^2 + R^2 = 1.0) */
static inline void sk_pan_equal_power(float pan, float *left, float *right) {
    pan = sk_clampf(pan, -1.0f, 1.0f);
    float angle = (pan + 1.0f) * (0.25f * (float)M_PI);
    if (left) *left = cosf(angle);
    if (right) *right = sinf(angle);
}

/* Linear stereo panning law */
static inline void sk_pan_linear(float pan, float *left, float *right) {
    pan = sk_clampf(pan, -1.0f, 1.0f);
    if (left) *left = (1.0f - pan) * 0.5f;
    if (right) *right = (1.0f + pan) * 0.5f;
}

static inline double sk_hz_to_phase_inc(double hz, double sr) {
    return (sr > 0.0) ? (2.0 * M_PI * hz / sr) : 0.0;
}

static inline double sk_ms_to_samples(double ms, double sr) {
    return ms * 0.001 * sr;
}

static inline double sk_samples_to_ms(double samples, double sr) {
    return (sr > 0.0) ? (samples * 1000.0 / sr) : 0.0;
}

/* =========================================================================
 * 3. PRNG & Audio Noise Generators (808/606 & Retro Hardware LFSRs)
 * ========================================================================= */

/* 64-bit Knuth MMIX Linear Congruential Generator */
typedef struct {
    uint64_t state;
} sk_rng_t;

static inline void sk_rng_init(sk_rng_t *rng, uint64_t seed) {
    rng->state = seed ? seed : 1ULL;
}

static inline uint64_t sk_rng_next(sk_rng_t *rng) {
    rng->state = rng->state * 6364136223846793005ULL + 1442695040888963407ULL;
    return rng->state;
}

static inline float sk_rng_float(sk_rng_t *rng) {
    uint32_t val = (uint32_t)(sk_rng_next(rng) >> 32);
    return (float)((int32_t)val) / 2147483648.0f;
}

/* 808/606 1-bit XOR clock shift register noise */
static inline float sk_metallic_noise_step(uint32_t clock) {
    uint32_t hh = (clock * 13) ^ (clock >> 5) ^ (clock * 193);
    return (hh & 128) ? 0.7f : -0.7f;
}

/* 6-oscillator metallic cluster buzz (ratios: 2.43, 3.01, 3.52, 4.11, 5.23, 6.78) */
static inline double sk_metallic_cluster_sample(double phase_inc, int sample_idx) {
    static const double ff[6] = {2.43, 3.01, 3.52, 4.11, 5.23, 6.78};
    double ss = 0.0;
    for (int j = 0; j < 6; j++) {
        ss += (sin((double)sample_idx * phase_inc * ff[j]) > 0.0) ? 1.0 : -1.0;
    }
    return ss / 6.0;
}

/* Maximum feedback taps for 2..31 bit LFSR polynomials */
static const uint8_t sk_max_lfsr_taps[32] = {
    0,  0,  1,  1,  1,  2,  1,  1,  2,  4,
    3,  2,  1,  1,  1,  1,  4,  3,  7,  1,
    3,  2,  1,  5,  1,  3,  1,  2,  3,  2,
    1,  3
};

/* Vintage Hardware LFSR Noise State (SID, POKEY, NES, Custom) */
typedef struct {
    uint32_t lfsr;
    int type;     /* 0=SID, 1..4=POKEY (4/5/9/17-bit), 5=NES 15-bit, 6=NES 7-bit, 7=Custom */
    int bits;     /* 2..31 */
    int tap;      /* feedback tap */
    int dac;      /* 0=1-bit (-1.0f or +1.0f), 1=8-bit DAC */
} sk_lfsr_noise_t;

static inline void sk_lfsr_noise_init(sk_lfsr_noise_t *n, int type) {
    n->type = sk_clampi(type, 0, 7);
    n->lfsr = (n->type == 0) ? 0x7FFFFF : 1;
    n->dac = 0;
    switch (n->type) {
        case 0: n->bits = 23; n->tap = 17; n->dac = 1; break;
        case 1: n->bits = 4;  n->tap = 2; break;
        case 2: n->bits = 5;  n->tap = 2; break;
        case 3: n->bits = 9;  n->tap = 4; break;
        case 4: n->bits = 17; n->tap = 4; break;
        case 5: n->bits = 15; n->tap = 1; break;
        case 6: n->bits = 15; n->tap = 6; break;
        case 7: n->bits = 16; n->tap = 4; break;
    }
}

static inline void sk_lfsr_noise_set_custom(sk_lfsr_noise_t *n, int bits, int tap, int dac) {
    if (bits < 2) bits = 2;
    if (bits > 31) bits = 31;
    if (tap < 1 || tap >= bits) tap = sk_max_lfsr_taps[bits];
    n->type = 7;
    n->bits = bits;
    n->tap = tap;
    n->dac = (dac != 0) ? 1 : 0;
    if (n->lfsr == 0) n->lfsr = 1;
}

static inline float sk_lfsr_noise_step(sk_lfsr_noise_t *n) {
    uint32_t lfsr = n->lfsr;
    int mode = n->type;
    if (lfsr == 0) {
        lfsr = (mode == 0) ? 0x7FFFFF : 1;
    }
    float out = 0.0f;
    switch (mode) {
        case 0: { /* MOS 6581/8580 SID 23-bit */
            uint32_t feedback = ((lfsr >> 22) ^ (lfsr >> 17)) & 1;
            lfsr = ((lfsr << 1) | feedback) & 0x7FFFFF;
            uint8_t dac = (uint8_t)(
                (((lfsr >> 22) & 1) << 7) |
                (((lfsr >> 20) & 1) << 6) |
                (((lfsr >> 16) & 1) << 5) |
                (((lfsr >> 13) & 1) << 4) |
                (((lfsr >> 11) & 1) << 3) |
                (((lfsr >>  7) & 1) << 2) |
                (((lfsr >>  4) & 1) << 1) |
                (((lfsr >>  2) & 1) << 0)
            );
            out = (float)dac * 0.00784313725f - 1.0f;
            break;
        }
        case 1: { /* POKEY 4-bit (15-step) */
            uint32_t feedback = ((lfsr >> 3) ^ (lfsr >> 2)) & 1;
            lfsr = ((lfsr << 1) | feedback) & 0x0F;
            out = (lfsr & 1) ? 1.0f : -1.0f;
            break;
        }
        case 2: { /* POKEY 5-bit (31-step) */
            uint32_t feedback = ((lfsr >> 4) ^ (lfsr >> 2)) & 1;
            lfsr = ((lfsr << 1) | feedback) & 0x1F;
            out = (lfsr & 1) ? 1.0f : -1.0f;
            break;
        }
        case 3: { /* POKEY 9-bit (511-step) */
            uint32_t feedback = ((lfsr >> 8) ^ (lfsr >> 4)) & 1;
            lfsr = ((lfsr << 1) | feedback) & 0x1FF;
            out = (lfsr & 1) ? 1.0f : -1.0f;
            break;
        }
        case 4: { /* POKEY 17-bit (131071-step) */
            uint32_t feedback = ((lfsr >> 16) ^ (lfsr >> 4)) & 1;
            lfsr = ((lfsr << 1) | feedback) & 0x1FFFF;
            out = (lfsr & 1) ? 1.0f : -1.0f;
            break;
        }
        case 5: { /* NES 15-bit long */
            uint32_t feedback = (lfsr ^ (lfsr >> 1)) & 1;
            lfsr = ((lfsr >> 1) | (feedback << 14)) & 0x7FFF;
            out = (lfsr & 1) ? 1.0f : -1.0f;
            break;
        }
        case 6: { /* NES 7-bit metallic loop */
            uint32_t feedback = (lfsr ^ (lfsr >> 6)) & 1;
            lfsr = ((lfsr >> 1) | (feedback << 14)) & 0x7FFF;
            out = (lfsr & 1) ? 1.0f : -1.0f;
            break;
        }
        case 7:
        default: {
            int bits = sk_clampi(n->bits, 2, 31);
            int tap = n->tap;
            if (tap < 1 || tap >= bits) tap = sk_max_lfsr_taps[bits];
            uint32_t mask = (bits >= 31) ? 0x7FFFFFFF : ((1U << bits) - 1);
            uint32_t feedback = ((lfsr >> (bits - 1)) ^ (lfsr >> tap)) & 1;
            lfsr = ((lfsr << 1) | feedback) & mask;
            if (n->dac == 1) {
                uint8_t dac = (uint8_t)(lfsr & 0xFF);
                out = (float)dac * 0.00784313725f - 1.0f;
            } else {
                out = (lfsr & 1) ? 1.0f : -1.0f;
            }
            break;
        }
    }
    n->lfsr = lfsr;
    return out;
}

/* =========================================================================
 * 4. Complex Phasor Rotator & Modal Resonator Bank
 * ========================================================================= */

/* Damped complex phasor rotator for modal synthesis (damped sinusoid) */
typedef struct {
    double re, im;     /* Current complex phasor state */
    double c, sn;      /* cos(w), sin(w) step increments */
    double decay;      /* exponential decay per sample (1.0 = sustain, <1.0 = decay) */
    double amp;        /* mode amplitude */
} sk_rotator_t;

static inline void sk_rotator_init(sk_rotator_t *r, double freq_hz, double amp, double t60_sec, double sr) {
    r->re = 1.0;
    r->im = 0.0;
    r->amp = amp;
    if (freq_hz <= 0.0 || freq_hz >= 0.5 * sr || sr <= 0.0) {
        r->c = 1.0; r->sn = 0.0; r->decay = 0.0; r->amp = 0.0;
        return;
    }
    double w = 2.0 * M_PI * freq_hz / sr;
    r->c = cos(w);
    r->sn = sin(w);
    r->decay = (t60_sec > 0.0) ? pow(10.0, -3.0 / (t60_sec * sr)) : 1.0;
}

static inline double sk_rotator_step(sk_rotator_t *r) {
    double out = r->amp * r->im;
    double nre = r->decay * (r->re * r->c - r->im * r->sn);
    double nim = r->decay * (r->re * r->sn + r->im * r->c);
    r->re = nre;
    r->im = nim;
    return out;
}

/* Modal bank renderer: renders sum of N damped sinusoids into an output buffer */
static inline void sk_modal_bank_render(const double *modes, int num_modes, double *out, int num_samples, double sr) {
    if (!modes || num_modes <= 0 || !out || num_samples <= 0) return;
    for (int i = 0; i < num_samples; i++) out[i] = 0.0;

    for (int j = 0; j < num_modes; j++) {
        double f = modes[3 * j];
        double amp = modes[3 * j + 1];
        double t60 = modes[3 * j + 2];
        if (f <= 0.0 || f >= 0.5 * sr) continue;

        sk_rotator_t rot;
        sk_rotator_init(&rot, f, amp, t60, sr);
        for (int i = 0; i < num_samples; i++) {
            out[i] += sk_rotator_step(&rot);
        }
    }
}

/* =========================================================================
 * 5. Filters: Chamberlin SVF, Biquad DF-II, 2-Pole Resonant, 1-Pole RC, Delay Filters
 * ========================================================================= */

/* Chamberlin Digital State-Variable Filter (SVF) */
typedef struct {
    double s0, s1;         /* Lowpass, Bandpass states */
    double f_coeff, damp;  /* Tuning coefficients */
} sk_svf_t;

static inline void sk_svf_init(sk_svf_t *f, double freq_hz, double q, double sr) {
    f->s0 = 0.0;
    f->s1 = 0.0;
    double q_val = (q < 0.01) ? 0.01 : q;
    f->damp = 1.0 / q_val;
    f->f_coeff = 2.0 * sin(M_PI * freq_hz / sr);
    if (f->f_coeff > 1.99) f->f_coeff = 1.99;
}

static inline void sk_svf_step(sk_svf_t *f, double in, double *lp, double *bp, double *hp) {
    double high = in - f->s0 - f->damp * f->s1;
    f->s1 += f->f_coeff * high;
    f->s0 += f->f_coeff * f->s1;
    f->s0 = sk_safe_val(f->s0);
    f->s1 = sk_safe_val(f->s1);
    if (hp) *hp = high;
    if (bp) *bp = f->s1;
    if (lp) *lp = f->s0;
}

static inline double sk_svf_step_bp(sk_svf_t *f, double in) {
    double bp;
    sk_svf_step(f, in, NULL, &bp, NULL);
    return bp;
}

static inline double sk_svf_step_lp(sk_svf_t *f, double in) {
    double lp;
    sk_svf_step(f, in, &lp, NULL, NULL);
    return lp;
}

/* Robert Bristow-Johnson (RBJ) Audio EQ Cookbook Direct Form II Biquad */
typedef enum {
    SK_BIQUAD_LOWPASS,
    SK_BIQUAD_HIGHPASS,
    SK_BIQUAD_BANDPASS,
    SK_BIQUAD_NOTCH,
    SK_BIQUAD_ALLPASS,
    SK_BIQUAD_LP_BP
} sk_biquad_type_t;

typedef struct {
    float b0, b1, b2, a1, a2;
    float x1, x2, y1, y2;
    float drive;
} sk_biquad_t;

static inline void sk_biquad_init(sk_biquad_t *b, sk_biquad_type_t type, float f, float q, float sr) {
    b->x1 = b->x2 = b->y1 = b->y2 = 0.0f;
    b->drive = 0.0f;
    if (sr <= 0.0f || f <= 0.0f) {
        b->b0 = 1.0f; b->b1 = b->b2 = b->a1 = b->a2 = 0.0f;
        return;
    }
    float omega = 2.0f * (float)M_PI * f / sr;
    float sin_omega = sinf(omega);
    float cos_omega = cosf(omega);
    float q_val = q < 0.01f ? 0.01f : q;
    float alpha = sin_omega / (2.0f * q_val);

    float a0 = 1.0f, a1 = 0.0f, a2 = 0.0f, b0 = 1.0f, b1 = 0.0f, b2 = 0.0f;

    switch (type) {
        case SK_BIQUAD_LOWPASS:
            b0 = (1.0f - cos_omega) * 0.5f;
            b1 = 1.0f - cos_omega;
            b2 = (1.0f - cos_omega) * 0.5f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;
        case SK_BIQUAD_HIGHPASS:
            b0 = (1.0f + cos_omega) * 0.5f;
            b1 = -(1.0f + cos_omega);
            b2 = (1.0f + cos_omega) * 0.5f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;
        case SK_BIQUAD_BANDPASS:
            b0 = alpha;
            b1 = 0.0f;
            b2 = -alpha;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;
        case SK_BIQUAD_NOTCH:
            b0 = 1.0f;
            b1 = -2.0f * cos_omega;
            b2 = 1.0f;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;
        case SK_BIQUAD_ALLPASS:
            b0 = 1.0f - alpha;
            b1 = -2.0f * cos_omega;
            b2 = 1.0f + alpha;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;
        case SK_BIQUAD_LP_BP:
            b0 = ((1.0f - cos_omega) * 0.5f) + alpha;
            b1 = 1.0f - cos_omega;
            b2 = ((1.0f - cos_omega) * 0.5f) - alpha;
            a0 = 1.0f + alpha;
            a1 = -2.0f * cos_omega;
            a2 = 1.0f - alpha;
            break;
    }

    float inv_a0 = 1.0f / a0;
    b->b0 = b0 * inv_a0;
    b->b1 = b1 * inv_a0;
    b->b2 = b2 * inv_a0;
    b->a1 = a1 * inv_a0;
    b->a2 = a2 * inv_a0;
}

static inline float sk_biquad_step(sk_biquad_t *b, float in) {
    float out = b->b0 * in + b->b1 * b->x1 + b->b2 * b->x2 - b->a1 * b->y1 - b->a2 * b->y2;
    b->x2 = b->x1;
    b->x1 = in;
    b->y2 = b->y1;
    b->y1 = out;
    return out;
}

/* 2-Pole Cascaded 1-Pole Resonant Lowpass Filter (vintage Moog style) */
typedef struct {
    double b0, b1;
    double ct, rs;
} sk_resonant2p_t;

static inline void sk_resonant2p_init(sk_resonant2p_t *f, double cutoff_norm, double resonance) {
    f->b0 = 0.0;
    f->b1 = 0.0;
    f->ct = (cutoff_norm > 0.95) ? 0.95 : (cutoff_norm < 0.0 ? 0.0 : cutoff_norm);
    f->rs = (resonance > 3.98) ? 3.98 : (resonance < 0.0 ? 0.0 : resonance);
}

static inline double sk_resonant2p_step(sk_resonant2p_t *f, double in) {
    double inp = in - (f->rs * f->b1);
    f->b0 += f->ct * (inp - f->b0);
    f->b1 += f->ct * (f->b0 - f->b1);
    f->b0 = sk_safe_val(f->b0);
    f->b1 = sk_safe_val(f->b1);
    return f->b1;
}

/* 1-Pole RC Lowpass / Damping Filter */
typedef struct {
    float y1;
    float coeff;
} sk_onepole_t;

static inline void sk_onepole_init_lp(sk_onepole_t *f, float fc, float sr) {
    f->y1 = 0.0f;
    if (sr <= 0.0f || fc >= sr * 0.5f) {
        f->coeff = 1.0f;
        return;
    }
    float x = expf(-2.0f * (float)M_PI * fc / sr);
    f->coeff = 1.0f - x;
}

static inline float sk_onepole_step(sk_onepole_t *f, float in) {
    f->y1 += f->coeff * (in - f->y1);
    return f->y1;
}

/* Logarithmic frequency mapping for delay feedback damping lowpass */
static inline float sk_delay_damping_coeff(float amount_norm, float sr) {
    amount_norm = sk_clampf(amount_norm, 0.0f, 1.0f);
    if (sr <= 0.0f) sr = 48000.0f;
    /* 20kHz (no damping) down to 500Hz (dark) */
    float fc = 20000.0f * powf(500.0f / 20000.0f, amount_norm);
    float x = expf(-2.0f * (float)M_PI * fc / sr);
    return 1.0f - x;
}

/* Logarithmic frequency mapping for delay feedback highpass (DC / bass buildup blocker) */
static inline float sk_delay_hp_coeff(float amount_norm, float sr) {
    amount_norm = sk_clampf(amount_norm, 0.0f, 1.0f);
    if (sr <= 0.0f) sr = 48000.0f;
    /* 20Hz (off) up to 2kHz (thin) */
    float fc = 20.0f * powf(2000.0f / 20.0f, amount_norm);
    float rc = 1.0f / (2.0f * (float)M_PI * fc);
    float dt = 1.0f / sr;
    return rc / (rc + dt);
}

/* =========================================================================
 * 6. Waveshaping, Distortion, Companding & Phase Warping
 * ========================================================================= */

/* Cubic soft-clipping with C1-continuous boundary at peak unity */
static inline float sk_softclip_cubic(float in, float drive) {
    if (drive <= 0.0f) return in;
    float d = in * drive;
    if (d > 1.0f) d = 1.0f;
    else if (d < -1.0f) d = -1.0f;
    return (d - (d * d * d) * (1.0f / 3.0f)) * 1.5f;
}

/* Simple stair-step quantizer function */
static inline float sk_quantize(float in, float levels) {
    if (levels <= 0.0f) return in;
    return floorf(in * levels) / levels;
}

static inline double sk_quantized(double in, double levels) {
    if (levels <= 0.0) return in;
    return floor(in * levels) / levels;
}

/* Bit-depth quantization with linear, companded (mu-law / tape), or dithered curves */
static inline float sk_quantize_bits_curve(float v, int bits, int curve, uint64_t *rng_state) {
    if (bits <= 0) return v;
    if (bits > 24) bits = 24;
    int levels = (1 << bits) - 1;
    if (levels < 1) levels = 1;

    if (curve == 1) {
        /* Cheap rational compander — boosts low-level detail before quantizing,
           un-boosts after: turns linear staircase into tape-style saturation. */
        const float k = 4.0f;
        float sign = v < 0.0f ? -1.0f : 1.0f;
        float av = fabsf(v);
        if (av > 1.0f) av = 1.0f;
        float companded = av * (1.0f + k) / (1.0f + k * av);
        int iv = (int)roundf(companded * (float)levels);
        float q = (float)iv * (1.0f / (float)levels);
        float expanded = q / (1.0f + k * (1.0f - q));
        return sign * expanded;
    }

    if (curve == 2 && rng_state) {
        /* Triangular dither before rounding (TPDF) */
        float t1 = sk_rng_float((sk_rng_t *)rng_state);
        float t2 = sk_rng_float((sk_rng_t *)rng_state);
        float dither = (t1 - t2) * (0.5f / (float)levels);
        int iv = (int)roundf((v + dither) * (float)levels);
        return (float)iv * (1.0f / (float)levels);
    }

    int iv = (int)roundf(v * (float)levels);
    return (float)iv * (1.0f / (float)levels);
}

/* Casio CZ-style Phase Distortion Warping */
static inline float sk_cz_phasor(int shape, float p, float d, float active_start, float active_end) {
    const float length = active_end - active_start;
    if (length <= 0.0f) return p;
    float phase = (p - active_start) / length;
    d = sk_clampf(d, -1.0f, 1.0f);
    const float amount = fabsf(d);

    switch (shape) {
        case 1: { /* saw -> pulse */
            const float breakpoint = 0.5f + 0.499f * d;
            const float inv_d = 0.5f / breakpoint;
            const float inv_1_minus_d = 0.5f / (1.0f - breakpoint);
            if (phase < breakpoint) {
                phase *= inv_d;
            } else {
                phase = 0.5f + (phase - breakpoint) * inv_1_minus_d;
            }
            break;
        }
        case 2: { /* square (folded sine) */
            const float half_d = d * 0.499f;
            const float scale = 0.5f / (0.5f - half_d);
            if (phase < 0.5f) {
                phase *= scale;
            } else {
                phase = 1.0f - (1.0f - phase) * scale;
            }
            break;
        }
        case 3: { /* triangle */
            const float half_d = d * 0.499f;
            const float scale = 0.5f / (0.5f - half_d);
            if (phase < 0.5f) {
                phase *= scale;
            } else {
                phase = 0.5f + (phase - 0.5f) * scale;
            }
            break;
        }
        case 4: { /* double sine */
            const float doubled = fmodf(phase * 2.0f, 1.0f);
            const float target = d < 0.0f ? 1.0f - doubled : doubled;
            phase += amount * (target - phase);
            break;
        }
        case 5: { /* saw -> triangle */
            const float half_d = d * 0.499f;
            const float scale1 = 0.5f / (0.5f - half_d);
            const float scale2 = 0.5f / (0.5f + half_d);
            if (phase < 0.5f) {
                phase *= scale1;
            } else {
                phase = 0.5f + (phase - 0.5f) * scale2;
            }
            break;
        }
        case 6: { /* resonant 1 */
            float exponent = 1.0f + 4.0f * amount;
            phase = sk_fast_pow(phase, d < 0.0f ? 1.0f / exponent : exponent);
            break;
        }
        case 7: { /* resonant 2 */
            float exponent = 1.0f + 8.0f * amount;
            phase = sk_fast_pow(phase, d < 0.0f ? 1.0f / exponent : exponent);
            break;
        }
        default:
            return p;
    }

    phase = sk_clampf(phase, 0.0f, 1.0f);
    return active_start + phase * length;
}

/* =========================================================================
 * 7. FFT & Spectral Analysis
 * ========================================================================= */

typedef struct { float re, im; } sk_cplx_t;

/* In-place Radix-2 Cooley-Tukey complex FFT/IFFT */
static inline void sk_fft_radix2(sk_cplx_t *buf, int n, int inverse) {
    if (!buf || n < 2) return;
    for (int i = 1, j = 0; i < n; i++) {
        int bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) {
            sk_cplx_t tmp = buf[i];
            buf[i] = buf[j];
            buf[j] = tmp;
        }
    }
    for (int len = 2; len <= n; len <<= 1) {
        float ang = (inverse ? 2.0f : -2.0f) * (float)M_PI / (float)len;
        sk_cplx_t wlen;
        wlen.re = cosf(ang);
        wlen.im = sinf(ang);
        for (int i = 0; i < n; i += len) {
            sk_cplx_t w = { 1.0f, 0.0f };
            for (int k = 0; k < len / 2; k++) {
                sk_cplx_t u = buf[i + k];
                sk_cplx_t t = buf[i + k + len / 2];
                sk_cplx_t v;
                v.re = t.re * w.re - t.im * w.im;
                v.im = t.re * w.im + t.im * w.re;
                buf[i + k].re         = u.re + v.re;
                buf[i + k].im         = u.im + v.im;
                buf[i + k + len/2].re = u.re - v.re;
                buf[i + k + len/2].im = u.im - v.im;
                float nwre = w.re * wlen.re - w.im * wlen.im;
                float nwim = w.re * wlen.im + w.im * wlen.re;
                w.re = nwre; w.im = nwim;
            }
        }
    }
    if (inverse) {
        float inv_n = 1.0f / (float)n;
        for (int i = 0; i < n; i++) {
            buf[i].re *= inv_n;
            buf[i].im *= inv_n;
        }
    }
}

/* =========================================================================
 * 8. Interpolation, Delay Line & Buffer Looping Tools
 * ========================================================================= */

static inline float sk_lerp(float a, float b, float frac) {
    return a + frac * (b - a);
}

static inline double sk_lerpd(double a, double b, double frac) {
    return a + frac * (b - a);
}

/* Linear interpolation read from a circular delay buffer */
static inline float sk_delay_read_linear(const float *buf, int buf_size, int write_pos, float delay_frames) {
    if (!buf || buf_size <= 2) return 0.0f;
    if (delay_frames < 1.0f) delay_frames = 1.0f;
    if (delay_frames > (float)(buf_size - 2)) delay_frames = (float)(buf_size - 2);

    float read = (float)write_pos - delay_frames;
    while (read < 0.0f) read += (float)buf_size;
    while (read >= (float)buf_size) read -= (float)buf_size;
    int i0 = (int)read;
    int i1 = i0 + 1;
    if (i1 >= buf_size) i1 = 0;
    float frac = read - (float)i0;
    return buf[i0] + frac * (buf[i1] - buf[i0]);
}

/* Zero-crossing detector with epsilon threshold */
static inline int sk_find_zero_crossings(const double *s, int n, double thresh, int *out_indices, int max_out) {
    if (!s || n < 2 || max_out <= 0) return 0;
    int cnt = 0;
    for (int i = 0; i < n - 1 && cnt < max_out; i++) {
        double v0 = s[i], v1 = s[i + 1];
        if ((v0 >= 0.0 && v1 < 0.0) || (v0 < 0.0 && v1 >= 0.0) || fabs(v0) <= thresh) {
            if (out_indices) out_indices[cnt] = i;
            cnt++;
        }
    }
    return cnt;
}

/* Loop crossfade boundary blend */
static inline void sk_crossfade_seam(double *s, int n, int start, int end, int xfade) {
    if (!s || n <= 0 || start < 0 || end > n) return;
    if (xfade > end - start) xfade = end - start;
    if (xfade < 1) return;
    for (int i = 0; i < xfade; i++) {
        double fade_out = (double)(xfade - i) / (double)xfade;
        double fade_in  = (double)i / (double)xfade;
        int ei = end - xfade + i;
        int si = start + i;
        if (ei >= 0 && ei < n && si >= 0 && si < n) {
            s[ei] = s[ei] * fade_out + s[si] * fade_in;
        }
    }
}

/* Glissando / Portamento parameter calculation:
 * mode: 0 = exponential/geometric, 1 = linear frequency, 2 = period-linear
 */
static inline void sk_glissando_calc(int mode, float from_inc, float target_inc, float frames, float *delta, float *speed) {
    if (!delta || !speed) return;
    *delta = 0.0f;
    *speed = 1.0f;
    if (frames <= 0.0f || from_inc <= 0.00001f || target_inc <= 0.00001f) return;

    if (mode == 2) {
        float p1 = 1.0f / from_inc;
        float p2 = 1.0f / target_inc;
        *delta = (p2 - p1) / frames;
    } else if (mode == 1) {
        *delta = (target_inc - from_inc) / frames;
    } else {
        *speed = powf(target_inc / from_inc, 1.0f / frames);
    }
}

/* Steps glissando/portamento frequency phase increment.
 * Returns 1 if gliding continues, 0 if target reached.
 */
static inline int sk_glissando_step(float *current_inc, float target_inc, int mode, float delta, float speed) {
    if (!current_inc) return 0;
    if (mode == 2) {
        float p = (1.0f / *current_inc) + delta;
        if (p <= 0.0f) {
            *current_inc = target_inc;
            return 0;
        }
        *current_inc = 1.0f / p;
        if ((delta > 0.0f && *current_inc <= target_inc) ||
            (delta < 0.0f && *current_inc >= target_inc)) {
            *current_inc = target_inc;
            return 0;
        }
        return 1;
    } else if (mode == 1) {
        *current_inc += delta;
        if ((delta > 0.0f && *current_inc >= target_inc) ||
            (delta < 0.0f && *current_inc <= target_inc)) {
            *current_inc = target_inc;
            return 0;
        }
        return 1;
    } else {
        if (speed == 1.0f) {
            *current_inc = target_inc;
            return 0;
        }
        *current_inc *= speed;
        if ((speed > 1.0f && *current_inc >= target_inc) ||
            (speed < 1.0f && *current_inc <= target_inc)) {
            *current_inc = target_inc;
            return 0;
        }
        return 1;
    }
}

#ifdef __cplusplus
}
#endif

#endif /* SK_DSP_H */
