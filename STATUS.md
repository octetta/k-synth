# k/synth Status

Last updated: 2026-03-29

## Web Sequencer (Current)

- Pad overlay includes:
  - 4x4 playable pad grid (`0..F`)
  - Drum-grid sequencer (4 rows x up to 16 steps)
  - Transport: `start`, `pause/resume` toggle, `stop`
  - Tempo control in quarter-note BPM
  - Step-count control (`1..16`)

- Sequencer rows are pad-referenced:
  - Each row selects a pad id (`0..F`)
  - Playback uses that pad's current slot and semitone configuration
  - Pad config now includes per-pad volume offset (`dB`)
  - Pad pitch supports floating-point semitone values (integers still work as semitone steps)

- Pattern state is mode-separated:
  - Drum mode has independent sequencer pattern data
  - Melodic mode has independent sequencer pattern data

- Live edit behavior:
  - Step toggles and row selector changes apply while running

## Session Save/Load

- Session JSON stores pattern data under `pattern.modes`:
  - `drum` and `melodic` each store:
    - `maxSteps`
    - `gridPads`
    - `grid`
- Backward compatibility:
  - Loader still accepts older single-pattern fields (`gridRows`, `grid`).
