# Melodex By Mark33s Beats Labs

Royal blue and black MIDI-playable, 16-voice VST3 synthesizer for Windows x64 / Mixcraft 10. Original preset-browser layout inspired by classic workstation plugins, with no Nexus artwork, code, or sound content.

## Get the Windows plugin
Open this pull request's **Checks**, then the successful **Build MELODEX Windows VST3** run. Download **MELODEX-Windows-x64-VST3** from Artifacts (GitHub sign-in required). Extract the ZIP; copy the entire `MELODEX.vst3` folder, including its Contents subfolder, into:

`C:\Program Files\Common Files\VST3\`

Administrator permission may be needed. Restart Mixcraft 10, rescan plugins in Preferences / Plug-Ins, and choose MELODEX on a virtual instrument track. Select your MIDI keyboard as the track input or draw notes in the piano roll. Audio comes from Melodex itself; it is an instrument, not a MIDI pattern generator. Keep your previous plugin backed up before installing this development version.

## Factory sounds
Exactly **2,000 factory synthesis recipes**, arranged as 20 families of 100 variants: grand keys, electric keys, organs, bells, mallets, plucks, guitars, basses, subs, analog leads, digital leads, pads, atmospheres, strings, brass, winds, choirs, synth keys, textures, and FX. Every entry loads distinct synthesis settings, not an empty name slot. These are synthetic interpretations and procedural variations; they are not 2,000 recorded instruments or individually auditioned premium sample patches. Eight harmonic oscillator types, dual oscillator detune, tone filtering, ADSR, stereo motion, ping-pong echo, reverb, and output gain provide the sound engine.

Browse categories or search names. Previous/next traverses the full bank. Click the keys to audition, or play a MIDI keyboard. MIDI sustain and ±2-semitone pitch bend use JUCE's synthesizer voice handling. Project state saves the selected preset and edited parameters. Preset changes preserve output gain. The legacy parameter ID `Phaser` now controls stereo motion, and old 0.1 envelope ranges/program indices differ; use this version in new projects.

## Build and verify
Requires CMake 3.22+, Visual Studio 2022 with Desktop development with C++, and internet to fetch JUCE 8.0.10.

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target MELODEX_VST3 preset_tests plugin_tests --parallel 2
ctest --test-dir build -C Release --output-on-failure
```

The workflow compiles x64 VST3, tests preset uniqueness/bounds, MIDI onset and audible output, state recall and release decay, then packages the plugin. It also exports an actual editor snapshot as **Melodex-interface-preview**. No Mixcraft host test or listening evaluation has been performed by CI. Test CPU usage, sustained chords, project reopen, and automation in your Mixcraft setup before production use. The additive engine's CPU usage scales with polyphony.

Uses JUCE under its applicable license; review JUCE licensing before distributing binaries commercially. No third-party sample library is bundled. This source revision is a development build, not a signed installer.
