#ifndef SIMPLE_AUDIO_H
#define SIMPLE_AUDIO_H

#include "miniaudio.h"

typedef struct {
    ma_engine engine;
} SimpleAudioContext;

/* ====================== Seamless Wavetable Voice (custom data source) ====================== */
typedef struct {
    ma_data_source_base ds;
    const float*        pTable;      /* mono wavetable, can be changed at runtime */
    ma_uint64           tableSize;
    ma_uint64           cursor;
    ma_uint32           channels;
} SimpleWavetableDataSource;

typedef struct {
    SimpleWavetableDataSource ds;
    ma_sound                  sound;
    bool                      is_initialized;
    float                     base_frequency;
    float                     base_volume;
    float                     base_pan;
} SimpleWavetableVoice;

/* LFO */
typedef enum {
    LFO_TARGET_PITCH,   /* vibrato */
    LFO_TARGET_VOLUME,  /* tremolo */
    LFO_TARGET_PAN
} SimpleAudioLFOTarget;

typedef struct {
    const float* wavetable;
    ma_uint64    tableSize;
    float        phase;
    float        frequency;
    float        depth;
    SimpleAudioLFOTarget target;
    bool         active;
} SimpleAudioLFO;

/* Engine */
ma_result simple_audio_init(SimpleAudioContext* ctx, float masterGain);  /* recommend 0.40f - 0.50f */
void simple_audio_uninit(SimpleAudioContext* ctx);
void simple_audio_set_master_volume(SimpleAudioContext* ctx, float volume);

/* Seamless wavetable voice */
ma_result simple_wavetable_create(SimpleAudioContext* ctx,
                                  const float* pInitialTable,   /* mono */
                                  ma_uint64 tableSize,
                                  float initialVolume,
                                  float baseFrequency,
                                  SimpleWavetableVoice* voice);

void simple_wavetable_play(SimpleWavetableVoice* voice);
void simple_wavetable_stop(SimpleWavetableVoice* voice);
void simple_wavetable_set_volume(SimpleWavetableVoice* voice, float volume);
void simple_wavetable_set_pan(SimpleWavetableVoice* voice, float pan);
void simple_wavetable_set_frequency(SimpleWavetableVoice* voice, float newFrequency);
void simple_wavetable_reset_phase(SimpleWavetableVoice* voice);

/* Change wavetable at runtime - NO STOP, NO BUZZ, NO CLICK */
void simple_wavetable_set_table(SimpleWavetableVoice* voice, const float* pNewTable, ma_uint64 newSize);

void simple_wavetable_uninit(SimpleWavetableVoice* voice);

/* LFO */
ma_result simple_audio_lfo_init(SimpleAudioLFO* lfo,
                                const float* pWavetable,
                                ma_uint64 tableSize,
                                float frequencyHz,
                                float depth,
                                SimpleAudioLFOTarget target);

void simple_audio_lfo_set_frequency(SimpleAudioLFO* lfo, float frequencyHz);
void simple_audio_lfo_set_depth(SimpleAudioLFO* lfo, float depth);
void simple_audio_lfo_set_active(SimpleAudioLFO* lfo, bool active);

/* Update LFO - call frequently (e.g. every 8ms) */
void simple_audio_lfo_update(SimpleAudioLFO* lfo, SimpleWavetableVoice* voice, float deltaTimeSeconds);

void simple_audio_lfo_deinit(SimpleAudioLFO* lfo);

#endif
