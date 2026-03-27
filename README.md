# Verdant Reverb — VST3 Plugin
## Build Guide

### What you need (all free)
| Tool | Download |
|------|----------|
| JUCE framework | https://juce.com/get-juce/ (clone or download) |
| CMake ≥ 3.22 | https://cmake.org/download/ |
| C++ compiler | Xcode (Mac) or Visual Studio 2022 (Windows) |

---

### Step 1 — Get JUCE

```bash
git clone https://github.com/juce-framework/JUCE.git
```

Place the `JUCE` folder **next to** the `ReverbDistortion` folder so the layout is:

```
Projects/
├── JUCE/
└── ReverbDistortion/
    ├── CMakeLists.txt
    └── Source/
        ├── PluginProcessor.h
        ├── PluginProcessor.cpp
        ├── PluginEditor.h
        └── PluginEditor.cpp
```

---

### Step 2 — Build on macOS

```bash
cd ReverbDistortion
cmake -B build -G Xcode
open build/ReverbDistortion.xcodeproj
# In Xcode: select ReverbDistortion_VST3 scheme → Product → Build
```

Or build from the command line:
```bash
cmake -B build
cmake --build build --config Release
```

The `.vst3` bundle appears at:
```
build/ReverbDistortion_artefacts/Release/VST3/Verdant Reverb.vst3
```

Copy it to `~/Library/Audio/Plug-Ins/VST3/`

---

### Step 3 — Build on Windows

Open **Developer Command Prompt for VS 2022**:

```bat
cd ReverbDistortion
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

The `.vst3` folder appears at:
```
build\ReverbDistortion_artefacts\Release\VST3\Verdant Reverb.vst3
```

Copy it to `C:\Program Files\Common Files\VST3\`

---

### Step 4 — Load in Ableton Live

1. Open Ableton Live
2. Preferences → Plug-ins → enable **VST3 Plug-In Custom Folder** (point to your VST3 folder, or it scans the default location automatically)
3. Click **Rescan**
4. Find **Verdant Reverb** in the plug-in browser under VST3
5. Drag onto any audio track

---

## Plugin Controls

### Reverb Section
| Knob | Description |
|------|-------------|
| ROOM | Room size (0 = small room, 1 = cathedral) |
| DAMP | High-frequency damping (simulates absorptive surfaces) |
| WIDTH | Stereo width of the reverb tail |
| WET | Level of the reverb signal |
| DRY | Level of the dry (original) signal |

### Distortion Section
| Control | Description |
|---------|-------------|
| DIST ON | Toggle button — enables distortion on the wet reverb signal only |
| DRIVE | Saturation amount (tanh soft-clip — stays musical even at high values) |
| DIST MIX | Blends between clean wet and distorted wet |
| TONE | Lowpass filter cutoff on the distorted wet (roll off harshness) |

---

## Tips
- High DRIVE + low TONE = warm, fuzzy reverb tails
- DIST MIX < 1.0 gives parallel distortion (cleaner shimmer)
- Stack with a pre-reverb compressor in Ableton for gated-reverb vibes
- Freeze mode can be added by exposing `reverbParams.freezeMode` — just add a toggle

---

## Customization ideas (for later)
- Pre-delay before the reverb (`juce::dsp::DelayLine`)
- Shimmer reverb (pitch-shift feedback inside the wet signal)
- Modulation (chorus/flanger on the wet tail)
- IR convolution reverb via `juce::dsp::Convolution`
"# STUKJE-VERB" 
