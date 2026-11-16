# SOUND EFECTRICKS — JUCE source project

SOUND EFECTRICKS is an original JUCE audio-effect prototype for Sound Era.
It builds AU, VST3 and Standalone targets. The CRYSTAL engine is an original
two-head granular pitch-delay concept; it is not a reproduction of proprietary
Soundtoys or Sugar Bytes code.

## Requirements on macOS Sequoia

- Xcode (install from the Mac App Store; launch once and accept its license)
- Xcode Command Line Tools: `xcode-select --install`
- CMake 3.22 or newer (`brew install cmake`)
- Git (`xcode-select --install` normally supplies it)
- Internet connection for the first CMake configure, which downloads JUCE 8.0.6

Install Homebrew from https://brew.sh if you do not already have it, then run:

```bash
brew install cmake
```

## Build

Open Terminal, enter the folder containing this project, then:

```bash
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release --parallel
```

The AU and VST3 bundles will be created under a path similar to:

```text
build/SoundEfectricks_artefacts/Release/AU/SOUND EFECTRICKS.component
build/SoundEfectricks_artefacts/Release/VST3/SOUND EFECTRICKS.vst3
```

Install for the current user:

```bash
mkdir -p "$HOME/Library/Audio/Plug-Ins/Components"
mkdir -p "$HOME/Library/Audio/Plug-Ins/VST3"
cp -R "build/SoundEfectricks_artefacts/Release/AU/SOUND EFECTRICKS.component" "$HOME/Library/Audio/Plug-Ins/Components/"
cp -R "build/SoundEfectricks_artefacts/Release/VST3/SOUND EFECTRICKS.vst3" "$HOME/Library/Audio/Plug-Ins/VST3/"
```

Then open Logic Pro > Settings > Plug-in Manager and rescan Audio Units.
VST3 is included for compatible hosts; Logic Pro uses AU.

## Current prototype contents

- 8 sequencer lanes × 32 steps, 1/16-note grid
- Host BPM / PPQ position following
- Crystal granular pitch-delay with pitch, grain, feedback, mix, spread and reverse
- Filter, phaser, bitcrush, stutter gate and reverb lanes
- APVTS parameters for host automation and parameter state
- AU, VST3 and Standalone CMake targets

## Important limitations of this first build

This is a functional starting prototype, not yet a commercial-release instrument.
LOOP and REVERSE lanes are currently reserved; the reverse switch affects the
Crystal grain reader. The grid pattern is saved with plugin state, but pattern
editing is not yet exposed as individual host-automatable parameters. The current
step selection is block-quantized rather than sample-accurate. The stutter lane
uses a rhythmic gate rather than a full audio looper. Further development should
add proper delay-buffer loop/reverse, sample-accurate step boundaries, oversampling,
denormal/CPU profiling, mono/stereo edge-case testing, preset management and
AU/VST3 validation on the target Mac.

## Notes

- `FORMATS AU VST3 Standalone` produces Audio Unit and VST3 builds.
- AU is the format Logic Pro loads. macOS AUv2 bundles use `.component`.
- For distribution outside your own Mac, plan code signing, notarization,
  validation with `auval`, and testing on each supported macOS/host version.
- If the CMake JUCE download fails, check Git/network access and retry configure.
