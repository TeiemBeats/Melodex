# MELODEX — Markees Beats Labs

Experimental MIDI-controlled synth VST3 for Windows x64 / Mixcraft 10, made with JUCE 8.

## GitHub Actions
Open **Actions → Build MELODEX Windows VST3 → Run workflow**. When successful, download the **MELODEX-Windows-x64-VST3** artifact, unzip it, and copy the `MELODEX.vst3` directory to `C:\Program Files\Common Files\VST3\`. Rescan plug-ins in Mixcraft 10.

The 4,000 entries are named placeholder program slots, **not 4,000 unique sampled instrument sounds**. Interface is currently an original neon-themed control layout, not a pixel-perfect reproduction of the concept artwork.

## Local build
```powershell
cmake -S . -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release --target MELODEX_VST3
```
Requires internet access to fetch JUCE. Source is provided as a development prototype; build and host validation are required.
