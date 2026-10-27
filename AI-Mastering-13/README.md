# AI Mastering 13 — custom VST3 source project

A custom mastering-effect concept requested under the name “Busy Works Beats · AI Mastering 13”, with metallic controls and dark panels inspired by the supplied interface reference. This is independent source code, not an official Busy Works Beats release or an existing commercial product. The screenshot's bird artwork is not included.

## What is delivered

C++17/JUCE source for a Windows x64 VST3 effect and standalone audio application; CMake project; double-click Windows build script; GitHub Actions workflow; DSP tests. **This archive does not contain a compiled .vst3.** The JUCE wrapper and GUI still require compilation and host testing on Windows. No Mixcraft compatibility certification is claimed.

The dependency is fetched from the JUCE 8.0.12 tag during CMake configuration. Git, an internet connection and development tools are required for the first build. JUCE is not bundled.

## Build on Windows

1. Install **Visual Studio 2022 Community** or **Build Tools 2022**, including “Desktop development with C++”, MSVC v143 and a Windows 10/11 SDK.
2. Install **CMake 3.22+** and **Git**, adding both to PATH.
3. Extract this archive to a writable local directory, then double-click `Build-Windows.cmd`.
4. Wait for configuration and compilation to finish. The resulting folder is:

   `build\AIMastering13_artefacts\Release\VST3\AI Mastering 13.vst3`

Alternatively, run these commands from this directory:

```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target AIMastering13_VST3 AIMastering13_Standalone
```

For a cloud build, put this directory's contents at the root of a GitHub repository, then run the **Build Windows VST3** workflow in Actions. Download and extract its Windows-x64 artifact. This workflow is supplied but has not been executed in this environment.

## Install in Mixcraft 10

1. Close Mixcraft.
2. Copy the **entire** built `AI Mastering 13.vst3` folder into:

   `C:\Program Files\Common Files\VST3\`

   Windows may request administrator permission. Do not rename source files to `.vst3`, and do not copy only the inner binary.
3. Reopen Mixcraft and let its plugin scan finish. Check **File > Plug-In Manager** if the effect does not appear; confirm it is enabled and not marked as failed to load.
4. Insert **AI Mastering 13** on the master bus, preferably after other mix effects. Start with `Default / neutral`. Play the track and compare presets with bypass at matched listening volume.

Mixcraft 10 supports third-party VST3 effects and is a 64-bit host. Official documentation: https://acoustica.com/mixcraft-10-manual/using-effects and https://acoustica.com/mixcraft-10-manual/plug-in-manager .

## Controls and processing

The GUI has 13 rotary controls, input gain, ceiling, bypass, four presets, output peak history and a level-based suggestion button. Host automation and session-state persistence are implemented for all audio parameters. Double-click a rotary control to reset it. Numeric boxes allow exact entry. The preset menu applies factory settings; current numeric settings, rather than the menu label, are authoritative after host recall or automation.

| Control | Range | Behavior |
|---|---|---|
| Clean | 0–100% | Blends in a 25 Hz one-pole high-pass/DC removal |
| Bass | -100–100% | Broad low band tilt, up to ±12 dB around 160 Hz |
| Tone | -100–100% | Broad high band tilt, up to ±9 dB around 3.2 kHz |
| Compress | 0–100% | Stereo-linked downward compression; threshold -6 to -30 dB, ratio 1:1 to 6:1 |
| Dynamic | 0–100% | Level-dependent attenuation of the high band |
| Shape | -100–100% | Envelope-based transient emphasis or softening |
| Excite | 0–100% | High band nonlinear harmonic coloration |
| Analog | 0–100% | Symmetric tanh saturation blend |
| Width | 0–200% | Mid/side width: mono at 0%, unchanged at 100% |
| Loud | 0–18 dB | Gain into the clipper and limiter |
| Clipper | 0–100% | Hard clip threshold: 0 to -6 dBFS; zero means 0 dBFS threshold |
| Limiter | 0–100% | Blends stereo-linked sample-peak limiting; 100% gives the set sample ceiling |
| Output | -24–6 dB | Additional gain before clipper/limiter, preserving the final ceiling |

Input range is -24 to +12 dB; ceiling is -12 to -0.1 dBFS, default -1 dBFS. The fixed DSP order is input → clean → bass/tone → transient shaping → dynamic HF attenuation → linked compression → exciter → saturation → width → loud/output gain → clipper → limiter → bypass crossfade. Controls smooth over about 20 ms; compressor attack is 12 ms with 150 ms release. Zero added latency; mono and stereo layouts supported. The standalone app's audio-device selection is provided by JUCE.

**Analyze & Suggest:** Play a representative loud section for 5–10 seconds, then press the button. A two-second exponentially weighted input RMS measurement and peak/crest estimate select gentle compression and up to 9 dB of drive toward roughly -16 dBFS RMS. It applies settings to all modules through host-notified parameters and replaces previous settings. This is a deterministic level heuristic, not a trained AI model, LUFS measurement, reference matcher, or spectral mastering engine. It cannot judge tonal balance. Silence is rejected. This first version does not include reference-file loading or module reordering.

The meter and history show **sample peaks in dBFS**, not LUFS or a spectrum. The limiter has instantaneous sample attack and 150 ms release; it is not oversampled and is not a true-peak limiter. Intersample peaks can exceed the sample ceiling. Reducing the limiter blend below 100% reduces ceiling protection. Clipping/saturation may alias at high drive. Mono compatibility should be checked when increasing width.

## Validation performed

The standalone DSP tests were compiled with GCC using C++17, `-O2 -Wall -Wextra -pedantic` and passed:

- Default neutral signal null below the limiter ceiling.
- Stereo sample ceiling under high drive at 22.05, 44.1, 48, 96 and 192 kHz.
- Bypass convergence, width-to-mono convergence, silence preservation.
- 300,000 samples of randomized parameter/input stability checks.

Re-run without JUCE:

```sh
g++ -std=c++17 -O2 -Wall -Wextra -pedantic -I Source tests/dsp_test.cpp -o dsp_test
./dsp_test
```

**Not yet validated:** full JUCE compilation, Windows binary loading, GUI rendering, Mixcraft plugin scanning, host automation/state recall, and audible listening tests. Before using it in a production session, build on Windows, test in an empty Mixcraft project, save/reopen a session with changed parameters, exercise bypass/automation, and compare render output with real-time playback. These are pending host checks, not completed results.

## Source and dependency licensing

Original project source is provided under the MIT license in `LICENSE`. JUCE has separate licensing terms; review https://github.com/juce-framework/JUCE/blob/8.0.12/LICENSE.md before distributing a built plugin. This project does not grant brand or trademark rights.
