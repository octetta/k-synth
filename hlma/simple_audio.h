/*
 * Updated simple_audio.h - Now supports BOTH:
 *   1. Fixed one-shot buffers (plays once, then stops)
 *   2. Wrapping buffers (wavetable / looping oscillator style)
 *
 * Everything else (volume, pan, pitch/frequency, real-time control) works exactly the same.
 */

#ifndef SIMPLE_AUDIO_H
#define SIMPLE_AUDIO_H

#include "miniaudio.h"

typedef struct {
    ma_engine engine;
} SimpleAudioContext;

typedef struct {
    ma_audio_buffer audio_buffer;
    ma_sound       sound;
    bool           is_initialized;
    bool           is_looping;          // for convenience
} SimpleAudioVoice;

/* Initialize the audio engine */
ma_result simple_audio_init(SimpleAudioContext* ctx);

/* Shut down */
void simple_audio_uninit(SimpleAudioContext* ctx);

/* Create a voice from a raw float32 PCM buffer.
 *   looping = false  → one-shot (plays once)
 *   looping = true   → wrapping / wavetable (loops forever, perfect for oscillators)
 *
 * The buffer you pass MUST stay alive until simple_audio_voice_uninit().
 */
ma_result simple_audio_create_voice(SimpleAudioContext* ctx,
                                    const float* pPCMData,
                                    ma_uint64 frameCount,
                                    ma_uint32 channels,
                                    float initialVolume,
                                    ma_bool32 looping,               // NEW: one-shot vs wrapping
                                    SimpleAudioVoice* voice);

/* Start playing (works for both one-shot and looping) */
void simple_audio_voice_play(SimpleAudioVoice* voice);

/* Stop immediately */
void simple_audio_voice_stop(SimpleAudioVoice* voice);

/* Real-time controls (all thread-safe) */
void simple_audio_voice_set_volume(SimpleAudioVoice* voice, float volume);
void simple_audio_voice_set_pan(SimpleAudioVoice* voice, float pan);

/* Real-time pitch/frequency control (works beautifully on looping voices) */
void simple_audio_voice_set_frequency(SimpleAudioVoice* voice, float newFrequency);

/* Toggle looping at runtime if you want (rarely needed) */
void simple_audio_voice_set_looping(SimpleAudioVoice* voice, ma_bool32 looping);

/* Clean up */
void simple_audio_voice_uninit(SimpleAudioVoice* voice);

#endif // SIMPLE_AUDIO_H

/* ======================== IMPLEMENTATION ======================== */
#ifdef SIMPLE_AUDIO_IMPLEMENTATION

ma_result simple_audio_init(SimpleAudioContext* ctx)
{
    ma_result result = ma_engine_init(NULL, &ctx->engine);
    return result;
}

void simple_audio_uninit(SimpleAudioContext* ctx)
{
    ma_engine_uninit(&ctx->engine);
}

ma_result simple_audio_create_voice(SimpleAudioContext* ctx,
                                    const float* pPCMData,
                                    ma_uint64 frameCount,
                                    ma_uint32 channels,
                                    float initialVolume,
                                    ma_bool32 looping,
                                    SimpleAudioVoice* voice)
{
    if (!ctx || !pPCMData || frameCount == 0 || channels == 0 || !voice) {
        return MA_INVALID_ARGS;
    }

    ma_audio_buffer_config bufferConfig = ma_audio_buffer_config_init(
        ma_format_f32, channels, frameCount, pPCMData, NULL);

    ma_result result = ma_audio_buffer_init(&bufferConfig, &voice->audio_buffer);
    if (result != MA_SUCCESS) return result;

    result = ma_sound_init_from_data_source(
        &ctx->engine,
        (ma_data_source*)&voice->audio_buffer,
        0,
        NULL,
        &voice->sound);

    if (result != MA_SUCCESS) {
        ma_audio_buffer_uninit(&voice->audio_buffer);
        return result;
    }

    ma_sound_set_volume(&voice->sound, initialVolume);
    ma_sound_set_looping(&voice->sound, looping);   // ← this is the key line

    voice->is_initialized = true;
    voice->is_looping = looping;
    return MA_SUCCESS;
}

void simple_audio_voice_play(SimpleAudioVoice* voice)
{
    if (voice && voice->is_initialized) ma_sound_start(&voice->sound);
}

void simple_audio_voice_stop(SimpleAudioVoice* voice)
{
    if (voice && voice->is_initialized) ma_sound_stop(&voice->sound);
}

void simple_audio_voice_set_volume(SimpleAudioVoice* voice, float volume)
{
    if (voice && voice->is_initialized) ma_sound_set_volume(&voice->sound, volume);
}

void simple_audio_voice_set_pan(SimpleAudioVoice* voice, float pan)
{
    if (voice && voice->is_initialized) ma_sound_set_pan(&voice->sound, pan);
}

void simple_audio_voice_set_frequency(SimpleAudioVoice* voice, float newFrequency)
{
    if (!voice || !voice->is_initialized) return;

    /* We assume the buffer was generated at BASE_FREQ (you pass this when creating the buffer).
     * For wavetable voices this gives perfect frequency control. */
    float pitch = newFrequency / 220.0f;   // BASE_FREQ = 220 Hz by convention in the example below
    ma_sound_set_pitch(&voice->sound, pitch);
}

void simple_audio_voice_set_looping(SimpleAudioVoice* voice, ma_bool32 looping)
{
    if (voice && voice->is_initialized) {
        ma_sound_set_looping(&voice->sound, looping);
        voice->is_looping = looping;
    }
}

void simple_audio_voice_uninit(SimpleAudioVoice* voice)
{
    if (voice && voice->is_initialized) {
        ma_sound_uninit(&voice->sound);
        ma_audio_buffer_uninit(&voice->audio_buffer);
        voice->is_initialized = false;
    }
}

#endif // SIMPLE_AUDIO_IMPLEMENTATION
