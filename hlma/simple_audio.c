#define SIMPLE_AUDIO_IMPLEMENTATION
#include "simple_audio.h"

#include <math.h>   /* for fmodf */

ma_result simple_audio_init(SimpleAudioContext* ctx, float masterGain)
{
    ma_result result = ma_engine_init(NULL, &ctx->engine);
    if (result == MA_SUCCESS) {
        ma_engine_set_volume(&ctx->engine, masterGain);   /* conservative gain to prevent clipping with many voices */
    }
    return result;
}

void simple_audio_uninit(SimpleAudioContext* ctx)
{
    ma_engine_uninit(&ctx->engine);
}

void simple_audio_set_master_volume(SimpleAudioContext* ctx, float volume)
{
    if (ctx) {
        ma_engine_set_volume(&ctx->engine, volume);
    }
}

ma_result simple_audio_create_voice(SimpleAudioContext* ctx,
                                    const float* pPCMData,
                                    ma_uint64 frameCount,
                                    ma_uint32 channels,
                                    float initialVolume,
                                    ma_bool32 looping,
                                    float baseFrequency,
                                    SimpleAudioVoice* voice)
{
    if (!ctx || !pPCMData || frameCount == 0 || channels == 0 || !voice) {
        return MA_INVALID_ARGS;
    }

    ma_audio_buffer_config cfg = ma_audio_buffer_config_init(ma_format_f32, channels, frameCount, pPCMData, NULL);
    ma_result result = ma_audio_buffer_init(&cfg, &voice->audio_buffer);
    if (result != MA_SUCCESS) return result;

    result = ma_sound_init_from_data_source(&ctx->engine, (ma_data_source*)&voice->audio_buffer, 0, NULL, &voice->sound);
    if (result != MA_SUCCESS) {
        ma_audio_buffer_uninit(&voice->audio_buffer);
        return result;
    }

    ma_sound_set_volume(&voice->sound, initialVolume);
    ma_sound_set_looping(&voice->sound, looping);

    voice->base_frequency = (baseFrequency > 0.0f) ? baseFrequency : 220.0f;
    voice->base_volume    = initialVolume;
    voice->base_pan       = 0.0f;
    voice->is_initialized = true;
    voice->is_looping     = looping;
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
        voice->base_volume = volume;
    }
}

void simple_audio_voice_set_pan(SimpleAudioVoice* voice, float pan)
{
    if (voice && voice->is_initialized) {
        ma_sound_set_pan(&voice->sound, pan);
        voice->base_pan = pan;
    }
}

void simple_audio_voice_set_frequency(SimpleAudioVoice* voice, float newFrequency)
{
    if (!voice || !voice->is_initialized || voice->base_frequency <= 0.0f) return;
    float pitch = newFrequency / voice->base_frequency;
    ma_sound_set_pitch(&voice->sound, pitch);
}

void simple_audio_voice_reset_phase(SimpleAudioVoice* voice)
{
    if (voice && voice->is_initialized) {
        ma_sound_seek_to_pcm_frame(&voice->sound, 0);
    }
}

ma_result simple_audio_voice_switch_wavetable(SimpleAudioVoice* voice,
                                              const float* pNewPCMData,
                                              ma_uint64 newFrameCount,
                                              float newBaseFrequency)
{
    if (!voice || !voice->is_initialized || !pNewPCMData || newFrameCount == 0) {
        return MA_INVALID_ARGS;
    }

    ma_bool32 wasPlaying = ma_sound_is_playing(&voice->sound);
    if (wasPlaying) ma_sound_stop(&voice->sound);

    ma_audio_buffer_uninit(&voice->audio_buffer);

    ma_audio_buffer_config cfg = ma_audio_buffer_config_init(ma_format_f32, 2, newFrameCount, pNewPCMData, NULL);
    ma_result result = ma_audio_buffer_init(&cfg, &voice->audio_buffer);
    if (result != MA_SUCCESS) return result;

    ma_sound_uninit(&voice->sound);
    result = ma_sound_init_from_data_source(ma_sound_get_engine(&voice->sound),
                                            (ma_data_source*)&voice->audio_buffer, 0, NULL, &voice->sound);

    if (result == MA_SUCCESS) {
        voice->base_frequency = newBaseFrequency;
        ma_sound_set_looping(&voice->sound, voice->is_looping);
        if (wasPlaying) ma_sound_start(&voice->sound);
    }
    return result;
}

void simple_audio_voice_uninit(SimpleAudioVoice* voice)
{
    if (voice && voice->is_initialized) {
        ma_sound_uninit(&voice->sound);
        ma_audio_buffer_uninit(&voice->audio_buffer);
        voice->is_initialized = false;
    }
}

/* LFO */
ma_result simple_audio_lfo_init(SimpleAudioLFO* lfo,
                                const float* pWavetable,
                                ma_uint64 tableSize,
                                float frequencyHz,
                                float depth,
                                SimpleAudioLFOTarget target)
{
    if (!lfo || !pWavetable || tableSize == 0) return MA_INVALID_ARGS;
    lfo->wavetable = pWavetable;
    lfo->tableSize = tableSize;
    lfo->phase     = 0.0f;
    lfo->frequency = frequencyHz;
    lfo->depth     = depth;
    lfo->target    = target;
    lfo->active    = true;
    return MA_SUCCESS;
}

void simple_audio_lfo_set_frequency(SimpleAudioLFO* lfo, float frequencyHz)
{
    if (lfo) lfo->frequency = frequencyHz;
}

void simple_audio_lfo_set_depth(SimpleAudioLFO* lfo, float depth)
{
    if (lfo) lfo->depth = depth;
}

void simple_audio_lfo_set_active(SimpleAudioLFO* lfo, bool active)
{
    if (lfo) lfo->active = active;
}

void simple_audio_lfo_update(SimpleAudioLFO* lfo, SimpleAudioVoice* voice, float deltaTimeSeconds)
{
    if (!lfo || !voice || !voice->is_initialized || !lfo->active || lfo->depth <= 0.0f) return;

    lfo->phase += lfo->frequency * deltaTimeSeconds;
    lfo->phase -= (float)(int)lfo->phase;   /* keep in [0,1) */

    float frac = lfo->phase * (float)lfo->tableSize;
    ma_uint64 i0 = (ma_uint64)frac;
    ma_uint64 i1 = (i0 + 1) % lfo->tableSize;
    float s = lfo->wavetable[i0] + (frac - (float)i0) * (lfo->wavetable[i1] - lfo->wavetable[i0]);

    if (lfo->target == LFO_TARGET_PITCH) {
        float modulated = voice->base_frequency * (1.0f + s * lfo->depth);
        simple_audio_voice_set_frequency(voice, modulated);
    } else if (lfo->target == LFO_TARGET_VOLUME) {
        float modulated = voice->base_volume * (1.0f + s * lfo->depth * 0.5f);  /* reduced sensitivity */
        simple_audio_voice_set_volume(voice, fmaxf(0.0f, modulated));
    } else if (lfo->target == LFO_TARGET_PAN) {
        float modulated = voice->base_pan + s * lfo->depth * 0.7f;
        simple_audio_voice_set_pan(voice, fmaxf(-1.0f, fminf(1.0f, modulated)));
    }
}

void simple_audio_lfo_deinit(SimpleAudioLFO* lfo)
{
    if (lfo) lfo->active = false;
}
