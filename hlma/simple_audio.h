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
    bool           is_looping;
    float          base_frequency;
    float          base_volume;
    float          base_pan;
} SimpleAudioVoice;

/* LFO target */
typedef enum {
    LFO_TARGET_PITCH,   /* vibrato / frequency modulation */
    LFO_TARGET_VOLUME,  /* tremolo */
    LFO_TARGET_PAN      /* pan modulation */
} SimpleAudioLFOTarget;

typedef struct {
    const float* wavetable;   /* user-owned mono float wavetable, values -1..1 recommended */
    ma_uint64    tableSize;
    float        phase;
    float        frequency;   /* LFO rate in Hz (can be high for "audio-rate" feel) */
    float        depth;       /* modulation amount */
    SimpleAudioLFOTarget target;
    bool         active;
} SimpleAudioLFO;

/* Engine functions */
ma_result simple_audio_init(SimpleAudioContext* ctx, float masterGain);  /* masterGain 0.4-0.6 recommended to avoid clipping */
void simple_audio_uninit(SimpleAudioContext* ctx);
void simple_audio_set_master_volume(SimpleAudioContext* ctx, float volume);

/* Voice functions - fixed buffer (one-shot) or wrapping wavetable (looping) */
ma_result simple_audio_create_voice(SimpleAudioContext* ctx,
                                    const float* pPCMData,      /* must remain valid until uninit */
                                    ma_uint64 frameCount,
                                    ma_uint32 channels,
                                    float initialVolume,
                                    ma_bool32 looping,          /* false = one-shot, true = looping wavetable */
                                    float baseFrequency,
                                    SimpleAudioVoice* voice);

void simple_audio_voice_play(SimpleAudioVoice* voice);
void simple_audio_voice_stop(SimpleAudioVoice* voice);
void simple_audio_voice_set_volume(SimpleAudioVoice* voice, float volume);
void simple_audio_voice_set_pan(SimpleAudioVoice* voice, float pan);
void simple_audio_voice_set_frequency(SimpleAudioVoice* voice, float newFrequency);
void simple_audio_voice_reset_phase(SimpleAudioVoice* voice);   /* restart from start of buffer (useful for looping voices) */

/* Switch to a new wavetable at runtime (for looping voices) */
ma_result simple_audio_voice_switch_wavetable(SimpleAudioVoice* voice,
                                              const float* pNewPCMData,
                                              ma_uint64 newFrameCount,
                                              float newBaseFrequency);

void simple_audio_voice_uninit(SimpleAudioVoice* voice);

/* LFO functions - arbitrary wavetable LFO */
ma_result simple_audio_lfo_init(SimpleAudioLFO* lfo,
                                const float* pWavetable,
                                ma_uint64 tableSize,
                                float frequencyHz,
                                float depth,
                                SimpleAudioLFOTarget target);

void simple_audio_lfo_set_frequency(SimpleAudioLFO* lfo, float frequencyHz);
void simple_audio_lfo_set_depth(SimpleAudioLFO* lfo, float depth);
void simple_audio_lfo_set_active(SimpleAudioLFO* lfo, bool active);

/* Update LFO - call regularly in your main loop (deltaTime in seconds) */
void simple_audio_lfo_update(SimpleAudioLFO* lfo, SimpleAudioVoice* voice, float deltaTimeSeconds);

void simple_audio_lfo_deinit(SimpleAudioLFO* lfo);  /* no-op, wavetable is user-owned */

#endif /* SIMPLE_AUDIO_H */
