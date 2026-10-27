# Orbit33 By Mark33s Beats Labs

A separate Windows x64 standalone synthesizer and VST3 instrument for Mixcraft 10. The original royal-blue, graphite, and silver interface combines a patch browser with two synth layers, modulation, filter, amp envelope, and effects panels.

## Install and play
Download **Orbit33-Windows-x64-App-and-VST3** from a successful **Build Orbit33 Windows App and VST3** GitHub Actions run. GitHub sign-in is required for artifact downloads. Extract the ZIP.

- **Standalone:** run `Orbit33.exe`. Open Options / Audio-MIDI Settings to choose the audio output, driver, sample rate, and MIDI keyboard inputs. Click the on-screen keys to play without external MIDI hardware. If Windows reports a missing MSVC runtime, install Microsoft's Visual C++ 2015-2022 x64 Redistributable.
- **Mixcraft 10:** copy the complete `Orbit33.vst3` folder with its Contents subfolder to `C:\Program Files\Common Files\VST3\` (administrator access may be required). Restart Mixcraft, rescan plugins, and load Orbit33 on a virtual instrument track. Use a MIDI keyboard or piano-roll notes.

## Sound engine
16-voice, dual-layer additive synthesis with eight harmonic oscillator models per layer. Select Layer A or B to edit that oscillator's model, character and detune. The blend control mixes the layers; B Octave transposes layer B. A shared two-pole low-pass filter with feedback, ADSR amplitude envelope, LFO (filter, amplitude or pan target), transpose, pan, stereo motion, echo, reverb and output gain shape the result. Pitch bend covers +/-2 semitones; sustain is handled by JUCE's MIDI synthesizer. Host projects recall edited parameters and the selected patch. Preset changes preserve master gain.

The bank contains **14,000 distinct procedural synthesis recipes**: 20 families x 100 parameter variations x 7 editions. Names, oscillator choices and actual synthesis settings are generated deterministically. These are synthesized interpretations and variations, not 14,000 sampled instruments or individually auditioned commercial patches. No external samples are needed. Categories include keys, organs, bells, mallets, plucks, guitars, basses, leads, pads, atmospheres, strings, brass, winds, choirs, textures and FX.

Search names, filter by family, and use Random to select within the filtered results. Previous/next traverses the full bank. Most controls are shared between layers; only engine, character and detune switch with the A/B buttons. The waveform display shows the selected oscillator's unfiltered harmonic shape.

## Scope
This is an original Orbit33 development build using its own identity and generated sounds. It does not contain Spectrasonics code, artwork, branding, patches or sample content. It does not reproduce Omnisphere's engine or offer its sample/granular engine, eight-part multitimbrality, arpeggiator, hardware integration, or full modulation matrix. This milestone targets Windows standalone and VST3; AU, AAX and macOS binaries are not included.

## Build and checks
Requires CMake 3.22+, Visual Studio 2022 Desktop development with C++, and internet access to fetch JUCE 8.0.10.

```powershell
cmake -S Orbit33 -B build-orbit33 -G "Visual Studio 17 2022" -A x64
cmake --build build-orbit33 --config Release --target Orbit33_VST3 Orbit33_Standalone orbit33_presets orbit33_tests --parallel 2
ctest --test-dir build-orbit33 -C Release --output-on-failure
```

CI checks all 14,000 names and synthesis recipe bounds/uniqueness, finite oscillator output, MIDI onset, audio output, project-state recall, release decay, layer differences and sound from all 20 families. It captures an actual interface image. All 14,000 patches are also rendered with the same C4 note at velocity 0.8 for 4,096 stereo frames at 22,050 Hz; CI rejects silence, non-finite audio, and duplicate 16-bit audio fingerprints. This detects exact matching rendered previews; it does not prove perceptual uniqueness across notes, velocities, or the full decay. Similar-sounding variations are intentionally retained to keep the requested 14,000-preset count.

Mixcraft hosting, standalone audio-device routing, listening quality and CPU performance still require manual testing. The additive engine's CPU cost rises with polyphony.

JUCE has its own licensing requirements; review those before commercial distribution. This is an unsigned development package, not a signed installer. Orbit33 is isolated in its own folder and does not replace Melodex.
