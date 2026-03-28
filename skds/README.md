# Skred
**A Precision Audio Engine for Hackers and Synthesists.**

Skred is a minimalist digital signal processing (DSP) library built for developers who demand absolute phase integrity and deterministic pitch. While most digital engines rely on floating-point smoothing that "drifts" or "beats" against hardware references, Skred is designed with a "suckless" philosophy—prioritizing double-precision accuracy and a single-header-style footprint.

## Core Philosophies

* **Phase Perfection**: Uses double-precision linear ramping for frequency changes. This eliminates the "asymptotic lag" found in standard exponential smoothing.
* **Universal Data Sources**: In Skred, everything is a wavetable. Whether it’s a sine wave, a recorded kick drum, or a complex LFO shape, they are all governed by the same unified lookup logic.
* **Miniaudio Native**: Skred implements the `ma_data_source` interface, allowing you to drop it directly into a node graph.

## The Public Interface

| Function | Description |
| :--- | :--- |
| `skred_voice_init` | Initializes a voice with a buffer and binds it to the hardware sample rate. |
| `skred_voice_set_freq` | Sets target frequency with millisecond-accurate linear ramping. |
| `skred_voice_set_adsr` | Manages the Attack, Decay, Sustain, and Release state machine. |
| `skred_voice_set_lfo` | Assigns any wavetable as an LFO to modulate Freq, Vol, or Pan. |
| `skred_voice_trig` | Bypasses the ADSR for instantaneous one-shot triggering. |

## Wiring into the Miniaudio Graph

Skred is built for modularity. You don't just "play" a voice; you "wire" it.

### 1. Simple Node Attachment
Wrap your Skred voice in an `ma_data_source_node` so the graph can talk to it:
```c
ma_data_source_node_config cfg = ma_data_source_node_config_init(&my_skred_voice);
ma_data_source_node_init(p_graph, &cfg, NULL, &voice_node);

### 2. Creating a Bus (The Mixer Node)

To sum multiple voices into a single processing chain, use an ma_node as a summing bus. By default, miniaudio nodes sum all attached inputs.

```c
// Create a generic node to act as a Mixer/Bus
ma_node_config mixer_cfg = ma_node_config_init();
mixer_cfg.vtable = &ma_splitter_node_vtable; // Standard passthrough/summing
ma_node_init(p_graph, &mixer_cfg, NULL, &mixer_bus);

// Attach voices to the mixer
ma_node_attach_output_bus(&voice_node_1, 0, &mixer_bus, 0);
ma_node_attach_output_bus(&voice_node_2, 0, &mixer_bus, 0);

// Attach the mixer to an effect
ma_node_attach_output_bus(&mixer_bus, 0, &filter_node, 0);
```

### 3. Dynamic Routing

The route command in the REPL demonstrates real-time graph manipulation. By detaching and re-attaching the output bus of a voice node, you can move signals through different effect paths without stopping the audio thread.
Modular REPL Demo

The included main.c provides a real-time environment to test these connections.
Example: The Pulsing Filter-Sweep

```
v 0           # Select voice 0
wave 1        # Set to Square wave
adsr 500 200 0.8 1000
on            # Trigger the note
route filter  # Pipe it through the resonant filter
cut 400       # Sweep the filter down
route delay   # Switch the routing to the stereo delay
```

> Created for the hacker who values precision over abstraction.
