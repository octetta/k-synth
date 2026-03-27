#define SIMPLE_AUDIO_IMPLEMENTATION
#include "simple_audio.h"
#include <math.h>

/* Engine */
ma_result simple_audio_init(SimpleAudioContext* ctx, float masterGain)
{
    ma_result result = ma_engine_init(NULL, &ctx->engine);
    if (result == MA_SUCCESS) {
        ma_engine_set_volume(&ctx->engine, masterGain);
    }
    return result;
}

void simple_audio_uninit(SimpleAudioContext* ctx)
{
    ma_engine_uninit(&ctx->engine);
}

void simple_audio_set_master_volume(SimpleAudioContext* ctx, float volume)
{
    if (ctx) ma_engine_set_volume(&ctx->engine, volume);
}

/* Custom Wavetable Data Source */
static ma_result wavetable_ds_read(ma_data_source* pDataSource, void* pFramesOut, ma_uint64 frameCount, ma_uint64* pFramesRead)
{
    SimpleWavetableDataSource* ds = (SimpleWavetableDataSource*)pDataSource;
    if (!ds || !ds->pTable || ds->tableSize == 0) {
        if (pFramesRead) *pFramesRead = 0;
        return MA_SUCCESS;
    }

    float* out = (float*)pFramesOut;
    ma_uint64 framesDone = 0;

    while (framesDone < frameCount) {
        ma_uint64 remaining = frameCount - framesDone;
        ma_uint64 toCopy = ds->tableSize - ds->cursor;
        if (toCopy > remaining) toCopy = remaining;

        for (ma_uint64 i = 0; i < toCopy; ++i) {
            float s = ds->pTable[ds->cursor + i];
            out[(framesDone + i) * ds->channels + 0] = s;
            if (ds->channels == 2) out[(framesDone + i) * 2 + 1] = s;
        }

        ds->cursor += toCopy;
        if (ds->cursor >= ds->tableSize) ds->cursor = 0;
        framesDone += toCopy;
    }

    if (pFramesRead) *pFramesRead = framesDone;
    return MA_SUCCESS;
}

static ma_result wavetable_ds_seek(ma_data_source* pDataSource, ma_uint64 frameIndex)
{
    SimpleWavetableDataSource* ds = (SimpleWavetableDataSource*)pDataSource;
    if (ds) ds->cursor = frameIndex % ds->tableSize;
    return MA_SUCCESS;
}

static ma_result wavetable_ds_get_format(ma_data_source* pDataSource, ma_format* pFormat, ma_uint32* pChannels, ma_uint32* pSampleRate, ma_channel* pChannelMap, size_t channelMapCap)
{
    SimpleWavetableDataSource* ds = (SimpleWavetableDataSource*)pDataSource;
    if (pFormat) *pFormat = ma_format_f32;
    if (pChannels) *pChannels = ds->channels;
    if (pSampleRate) *pSampleRate = 48000;
    return MA_SUCCESS;
}

static ma_data_source_vtable g_wavetableVTable = {
    wavetable_ds_read,
    wavetable_ds_seek,
    wavetable_ds_get_format,
    NULL,
    NULL
};

/* Wavetable Voice */
ma_result simple_wavetable_create(SimpleAudioContext* ctx,
                                  const float* pInitialTable,
                                  ma_uint64 tableSize,
                                  float initialVolume,
                                  float baseFrequency,
                                  SimpleWavetableVoice* voice)
{
    if (!ctx || !pInitialTable || tableSize == 0 || !voice) return MA_INVALID_ARGS;

    ma_data_source_config dsConfig = ma_data_source_config_init();
    dsConfig.vtable = &g_wavetableVTable;

    ma_result result = ma_data_source_init(&dsConfig, &voice->ds);
    if (result != MA_SUCCESS) return result;

    voice->ds.pTable    = pInitialTable;
    voice->ds.tableSize = tableSize;
    voice->ds.cursor    = 0;
    voice->ds.channels  = 2;

    result = ma_sound_init_from_data_source(&ctx->engine, &voice->ds, 0, NULL, &voice->sound);
    if (result != MA_SUCCESS) {
        ma_data_source_uninit(&voice->ds);
        return result;
    }

    ma_sound_set_volume(&voice->sound, initialVolume);
    ma_sound_set_looping(&voice->sound, MA_TRUE);

    voice->base_frequency = (baseFrequency > 0.0f) ? baseFrequency : 220.0f;
    voice->base_volume    = initialVolume;
    voice->base_pan       = 0.0f;
    voice->is_initialized = true;
    return MA_SUCCESS;
}

void simple_wavetable_set_table(SimpleWavetableVoice* voice, const float* pNewTable, ma_uint64 newSize)
{
    if (voice && voice->is_initialized && pNewTable && newSize > 0) {
        voice->ds.pTable = pNewTable;
        voice->ds.tableSize = newSize;
    }
}

void simple_wavetable_play(SimpleWavetableVoice* voice)
{
    if (voice && voice->is_initialized) ma_sound_start(&voice->sound);
}

void simple_wavetable_stop(SimpleWavetableVoice* voice)
{
    if (voice && voice->is_initialized) ma_sound_stop(&voice->sound);
}

void simple_wavetable_set_volume(SimpleWavetableVoice* voice, float volume)
{
    if (voice && voice->is_initialized) {
        ma_sound_set_volume(&voice->sound, volume);
        voice->base_volume = volume;
    }
}

void simple_wavetable_set_pan(SimpleWavetableVoice* voice, float pan)
{
    if (voice && voice->is_initialized) {
        ma_sound_set_pan(&voice->sound, pan);
        voice->base_pan = pan;
    }
}

void simple_wavetable_set_frequency(SimpleWavetableVoice* voice, float newFrequency)
{
    if (voice && voice->is_initialized && voice->base_frequency > 0.0f) {
        ma_sound_set_pitch(&voice->sound, newFrequency / voice->base_frequency);
    }
}

void simple_wavetable_reset_phase(SimpleWavetableVoice* voice)
{
    if (voice && voice->is_initialized) ma_sound_seek_to_pcm_frame(&voice->sound, 0);
}

void simple_wavetable_uninit(SimpleWavetableVoice* voice)
{
    if (voice && voice->is_initialized) {
        ma_sound_uninit(&voice->sound);
        ma_data_source_uninit(&voice->ds);
        voice->is_initialized = false;
    }
}

/* LFO - Fixed for continuous repeating tremolo */
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

void simple_audio_lfo_update(SimpleAudioLFO* lfo, SimpleWavetableVoice* voice, float deltaTimeSeconds)
{
    if (!lfo || !voice || !voice->is_initialized || !lfo->active || lfo->depth <= 0.0f) return;

    lfo->phase += lfo->frequency * deltaTimeSeconds;
    while (lfo->phase >= 1.0f) lfo->phase -= 1.0f;

    float frac = lfo->phase * (float)lfo->tableSize;
    ma_uint64 i0 = (ma_uint64)frac;
    ma_uint64 i1 = (i0 + 1) % lfo->tableSize;
    float modSample = lfo->wavetable[i0] + (frac - (float)i0) * (lfo->wavetable[i1] - lfo->wavetable[i0]);

    if (lfo->target == LFO_TARGET_PITCH) {
        float modulated = voice->base_frequency * (1.0f + modSample * lfo->depth * 1.2f);
        simple_wavetable_set_frequency(voice, modulated);
    }
    else if (lfo->target == LFO_TARGET_VOLUME) {
        /* Continuous tremolo - always based on original base_volume */
        float modulated = voice->base_volume * (1.0f + modSample * lfo->depth * 0.9f);
        simple_wavetable_set_volume(voice, fmaxf(0.08f, modulated));
    }
    else if (lfo->target == LFO_TARGET_PAN) {
        float modulated = voice->base_pan + modSample * lfo->depth * 0.8f;
        simple_wavetable_set_pan(voice, fmaxf(-1.0f, fminf(1.0f, modulated)));
    }
}

void simple_audio_lfo_deinit(SimpleAudioLFO* lfo)
{
    if (lfo) lfo->active = false;
}
