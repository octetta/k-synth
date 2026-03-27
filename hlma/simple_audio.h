/*
 * simple_audio.h - Simplified C wrapper for MiniAudio high-level API
 * 
 * Purpose: Make it dead-simple to play floating-point PCM buffers as independent "voices"
 *          with per-voice real-time volume + panning control (and easy extension for filters).
 * 
 * Features:
 *   - Feed raw float32 interleaved PCM buffers (you own the memory).
 *   - Each voice is an independent ma_sound (full high-level mixing, no low-level device hassle).
 *   - Real-time volume (linear 0.0-∞) and pan (-1.0 left … +1.0 right).
 *   - Zero node-graph knowledge required for basic use.
 *   - Tiny API – just init, create_voice, play/stop, set_volume/set_pan, uninit.
 * 
 * Usage notes:
 *   - Your PCM buffer MUST be float32 interleaved.
 *   - Best results: match engine sample rate (default 48000) and channels (default 2).
 *     MiniAudio will resample automatically if they differ, but matching avoids extra CPU.
 *   - The buffer you pass must stay alive until simple_audio_voice_uninit() is called.
 *   - Call set_volume/set_pan from any thread – they are real-time safe.
 * 
 * For filters (low-pass, etc.): see the comment at the bottom of this file.
 */

#ifndef SIMPLE_AUDIO_H
#define SIMPLE_AUDIO_H

#include "miniaudio.h"   // <-- You must #define MINIAUDIO_IMPLEMENTATION in ONE .c file

typedef struct {
    ma_engine engine;
} SimpleAudioContext;

typedef struct {
    ma_audio_buffer audio_buffer;   // owns the data source
    ma_sound       sound;           // the voice itself
    bool           is_initialized;
} SimpleAudioVoice;

/* Initialize the audio engine (default device, 48kHz, stereo, f32). */
ma_result simple_audio_init(SimpleAudioContext* ctx);

/* Shut down the engine. */
void simple_audio_uninit(SimpleAudioContext* ctx);

/* Create a voice from a raw float32 PCM buffer.
 *   pPCMData   - pointer to your interleaved float32 samples (you own it)
 *   frameCount - number of frames (NOT bytes!)
 *   channels   - 1 or 2 (recommend matching engine channels)
 *   initialVolume - 0.0 = silent, 1.0 = normal, >1.0 = amplify
 * Returns MA_SUCCESS on success. */
ma_result simple_audio_create_voice(SimpleAudioContext* ctx,
                                    const float* pPCMData,
                                    ma_uint64 frameCount,
                                    ma_uint32 channels,
                                    float initialVolume,
                                    SimpleAudioVoice* voice);

/* Start playing the voice (can be called multiple times). */
void simple_audio_voice_play(SimpleAudioVoice* voice);

/* Stop the voice immediately. */
void simple_audio_voice_stop(SimpleAudioVoice* voice);

/* Real-time volume control (linear). Thread-safe. */
void simple_audio_voice_set_volume(SimpleAudioVoice* voice, float volume);

/* Real-time panning (-1.0 = full left, 0.0 = center, 1.0 = full right). Thread-safe. */
void simple_audio_voice_set_pan(SimpleAudioVoice* voice, float pan);

/* Clean up a voice (must be called before the PCM buffer is freed). */
void simple_audio_voice_uninit(SimpleAudioVoice* voice);

#endif // SIMPLE_AUDIO_H

/* ======================== IMPLEMENTATION ======================== */
/* Paste the code below into ONE .c file AFTER #define MINIAUDIO_IMPLEMENTATION */

#ifdef SIMPLE_AUDIO_IMPLEMENTATION

ma_result simple_audio_init(SimpleAudioContext* ctx)
{
    ma_result result = ma_engine_init(NULL, &ctx->engine);
    if (result != MA_SUCCESS) {
        return result;
    }
    return MA_SUCCESS;
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
                                    SimpleAudioVoice* voice)
{
    if (!ctx || !pPCMData || frameCount == 0 || channels == 0 || !voice) {
        return MA_INVALID_ARGS;
    }

    ma_audio_buffer_config bufferConfig = ma_audio_buffer_config_init(
        ma_format_f32,          // must be float32 for best performance
        channels,
        frameCount,
        pPCMData,               // external data – we do NOT copy
        NULL                    // no custom allocator
    );

    ma_result result = ma_audio_buffer_init(&bufferConfig, &voice->audio_buffer);
    if (result != MA_SUCCESS) {
        return result;
    }

    /* Create the sound (voice) from the audio buffer data source */
    result = ma_sound_init_from_data_source(
        &ctx->engine,
        (ma_data_source*)&voice->audio_buffer,   // ma_audio_buffer is compatible with ma_data_source
        0,                                       // default flags (you can add MA_SOUND_FLAG_NO_SPATIALIZATION if you want)
        NULL,                                    // no parent sound group
        &voice->sound
    );

    if (result != MA_SUCCESS) {
        ma_audio_buffer_uninit(&voice->audio_buffer);
        return result;
    }

    /* Initial volume */
    ma_sound_set_volume(&voice->sound, initialVolume);

    voice->is_initialized = true;
    return MA_SUCCESS;
}

void simple_audio_voice_play(SimpleAudioVoice* voice)
{
    if (voice && voice->is_initialized) {
        ma_sound_start(&voice->sound);
    }
}

void simple_audio_voice_stop(SimpleAudioVoice* voice)
{
    if (voice && voice->is_initialized) {
        ma_sound_stop(&voice->sound);
    }
}

void simple_audio_voice_set_volume(SimpleAudioVoice* voice, float volume)
{
    if (voice && voice->is_initialized) {
        ma_sound_set_volume(&voice->sound, volume);
    }
}

void simple_audio_voice_set_pan(SimpleAudioVoice* voice, float pan)
{
    if (voice && voice->is_initialized) {
        ma_sound_set_pan(&voice->sound, pan);
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

/*
 * ======================== EXAMPLE USAGE ========================
 * 
 * In your main .c file:
 * 
 * #define MINIAUDIO_IMPLEMENTATION
 * #define SIMPLE_AUDIO_IMPLEMENTATION
 * #include "miniaudio.h"
 * #include "simple_audio.h"
 * 
 * int main(void)
 * {
 *     SimpleAudioContext ctx;
 *     simple_audio_init(&ctx);
 * 
 *     // Example: 1 second of 440 Hz sine wave at 48kHz, stereo
 *     #define SAMPLE_RATE 48000
 *     #define DURATION    1.0f
 *     #define FRAMES      (ma_uint64)(SAMPLE_RATE * DURATION)
 *     float* sineBuffer = malloc(FRAMES * 2 * sizeof(float)); // stereo
 *     for (ma_uint64 i = 0; i < FRAMES; ++i) {
 *         float sample = (float)sin(2.0 * M_PI * 440.0 * i / SAMPLE_RATE);
 *         sineBuffer[i*2 + 0] = sample; // left
 *         sineBuffer[i*2 + 1] = sample; // right
 *     }
 * 
 *     SimpleAudioVoice voice;
 *     simple_audio_create_voice(&ctx, sineBuffer, FRAMES, 2, 1.0f, &voice);
 * 
 *     simple_audio_voice_play(&voice);
 *     simple_audio_voice_set_volume(&voice, 0.8f);
 *     simple_audio_voice_set_pan(&voice, -0.5f);   // bias left
 * 
 *     printf("Playing... press Enter to stop\n");
 *     getchar();
 * 
 *     simple_audio_voice_stop(&voice);
 *     simple_audio_voice_uninit(&voice);
 *     free(sineBuffer);
 * 
 *     simple_audio_uninit(&ctx);
 *     return 0;
 * }
 */

/*
 * ======================== ADDING FILTERS (OPTIONAL) ========================
 * If you want real-time filters (low-pass, etc.) per voice without learning the full node graph:
 * 
 * 1. After creating the voice, create a simple effect node (e.g. ma_lpf_node for low-pass).
 * 2. Insert it between the sound and the engine endpoint:
 *      ma_lpf_node lpf;
 *      ma_lpf_node_config lpfConfig = ma_lpf_node_config_init(
 *          ma_engine_get_channels(&ctx->engine),
 *          ma_engine_get_sample_rate(&ctx->engine),
 *          800.0f,   // cutoff frequency
 *          2         // order
 *      );
 *      ma_lpf_node_init(ma_engine_get_node_graph(&ctx->engine), &lpfConfig, NULL, &lpf);
 * 
 *      // Route: sound -> lpf -> engine endpoint
 *      ma_node_attach_output_bus(&voice->sound, 0, &lpf, 0);
 *      ma_node_attach_output_bus(&lpf, 0, ma_engine_get_endpoint(&ctx->engine), 0);
 * 
 * 3. To change cutoff in real-time: ma_lpf_node_set_cutoff(&lpf, newCutoff);
 * 
 * This is the minimal node-graph usage and stays very close to the simple API above.
 * You can wrap it further if you need many voices with filters.
 * 
 * Full engine_effects.html example on miniaud.io shows exactly this pattern.
 * 
 * Enjoy the audio!
 */
