# DJO Guitar Studio

A JUCE-based guitar effect plugin project inspired by the broad guitar textures of Djo's **"End of Beginning"** and **"Basic Being Basic."** It is an original effect chain, not an official Djo product and not an exact recreation of any studio recording.

## Presets
- **End of Beginning** — cleaner, wider, slower modulation, more delay and reverb.
- **Basic Being Basic** — more drive and presence, less modulation and ambience for a tighter rhythm sound.

## Effects and controls
Drive, Tone, Modulation, Mod Rate, Delay, Reverb, Mix, Output.

## Build
1. Install a recent JUCE release (JUCE 7 or 8) and CMake.
2. Clone this repository.
3. Place JUCE beside this folder as `../JUCE`, or configure with `-DJUCE_DIR=/path/to/JUCE`.
4. Build:
   ```bash
   cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
   cmake --build build --config Release
   ```
5. Open the generated AU/VST3 in Logic Pro or another compatible host. On macOS, enable the AU build and rescan plug-ins in Logic.

## Important
This is a starter implementation. It has not been compiled or tested against your specific JUCE installation or guitar/audio interface. Treat it as a starting point: audition at conservative levels, compare in context with the original tracks, and adjust the controls to taste. For best results, start with a clean amp or amp-sim input; disable other chorus/delay/reverb while dialing it in.

## GitHub
Create a new empty GitHub repository, unzip this project, then:
```bash
git init
git add .
git commit -m "Initial DJO Guitar Studio plugin"
git branch -M main
git remote add origin https://github.com/YOUR-USERNAME/DJO-Guitar-Studio.git
git push -u origin main
```
