# BREAD v0.1 — start here

An editable two-transistor silicon fuzz workbench, with a browser app and a native JUCE plugin project. The original README.md is the product vision; this file describes the implemented milestone.

## Run the browser app on your Mac

```bash
cd ~/Dropbox/PROJECTS/bread
bash run.sh
```

Requires Python 3; no Python packages, account, tokens, Node installation, or virtual environment are needed. It opens http://localhost:8765. Leave the Terminal window running; Ctrl+C stops the server. Only the public `dist` folder is served, and the server listens on your own computer only. Opening index.html directly from Finder will not load the audio worklet: use run.sh.

Click **Play demo** to hear the built-in synthesized bass loop through the circuit. **Live input** uses your browser's selected microphone/audio interface. Allow microphone access and use headphones to avoid feedback. **Audio file** loops a local recording. Audio stays in your browser. The browser version processes mono input and sends it to both output channels.

### Patch the circuit

- Click a component to inspect it. Enter a value and click Apply value.
- Resistors use ohms, capacitors use farads, NPNs use current gain β, and diodes use saturation current in amps.
- Click two pins to connect them, or drag from one pin to another.
- Drag a component body to reposition it.
- Select a patch lead, then Unplug end. The end hangs loose and the connection is actually removed from the audio circuit. Click the loose end and another pin to reconnect.
- Add a component from the drawer, wire its pins, and hear it enter the circuit.
- Undo restores the previous circuit, including after Reset. Save downloads your circuit as JSON; Open restores it.
- Your current circuit is also saved in this browser's local storage. This is device-local, not cloud sync. Export JSON to move it between devices or into the plugin.
- The board's holes are visual guides. They are not hidden electrical buses; explicit wires define every connection.

Try changing C3 from `1e-8` (10 nF) to `1e-7` (100 nF), lowering Q1's β from 180 to 60, or altering R3 to move Q2's bias. Input, Output, and Wet/Dry controls sit below the board.

The scope shows the final output waveform. The multimeter shows the selected node relative to ground: a settled operating-point estimate with audio stopped, or periodically sampled live voltage during playback. It is not a calibrated RMS meter.

Open supports the shared JSON circuit format and a SPICE subset: R, C, D, Q (NPN), P pots, and `.model` lines. D model names pick the junction (1N34A/1N60=germanium, BAT41=schottky, LED, else silicon); `.model NAME npn(bf=180)` sets transistor beta regardless of where it appears. Q nodes are SPICE order collector base emitter. SPICE net names `in`, `out`, `vcc`, and `0`/`gnd` map to the workbench terminals, and a `* Nodes: 0=Gnd, 1=In, 2=Out, 10=VCC` header maps numbered-node netlists; V sources and common directives (`.tran`, `.param`, `.ic`, ...) are ignored. Value units may keep their letters (2.2uF, 8.2k) and sub-1Ω wiper resistors clamp to the 1Ω floor. Lines beginning with `*` and `.end` are accepted. Paste a netlist straight from the web with **Paste netlist**, or open a `.cir`/`.sp`/`.txt` file. See `presets/fuzz-face.cir` for a working example. BJT stages need a DC path for every base: a classic positive-ground PNP circuit pasted as NPN+ground can sit silent until Q1 gets a collector load to VCC and a base→emitter bias; `presets/fuzz-face-npn.cir` shows the exact two parts and why. Other SPICE devices (inductors, MOSFETs, subcircuits) are rejected rather than silently dropped.

## Build VST3, AU, and standalone on your M1 Pro

Install Apple's command line tools and CMake if they are not already present:

```bash
xcode-select --install
brew install cmake
```

Then:

```bash
cd ~/Dropbox/PROJECTS/bread
bash build-native.sh
```

The first build downloads the pinned JUCE 8.0.6 source revision from GitHub. Subsequent builds reuse it. The default Mac build targets your Mac's architecture (arm64 on an M1). Output is under `build/Bread_artefacts/Release/`:

- `VST3/BREAD.vst3`
- `AU/BREAD.component`
- `Standalone/BREAD.app`

Open the standalone app from Finder and select your interface in its audio settings. Its default feedback protection may mute input until you explicitly enable it.

To install the built VST3 and AU into your user plugin folders:

```bash
bash install-native.sh
```

Restart/rescan your DAW and put BREAD on an audio track. Host automation is available for Input, Output, and Mix. Circuit topology and component values are saved in DAW state. The native editor supports adding R/C/NPN/diode parts, dragging components, editing SI values with Enter, JSON open/save, and wiring by clicking pins. Right-click a pin to unplug one attached wire; select two pins to create a new connection. Its interface is simpler than the browser workbench. Use the browser for the scope, node probe, Undo, BOM, and SPICE import.

## What is real in this milestone

Both implementations solve the wire graph electrically. There is no hidden prewritten fuzz effect behind the component pictures. Resistors stamp conductance; capacitors use backward-Euler companion models; diodes use exponential junction curves; NPNs use an Ebers–Moll model with variable forward β. Ground, input, and a fixed 9 V source are boundary nodes. A bounded, damped Newton solver with pivoted elimination updates node voltages for each sample. Tiny 1 GΩ paths to ground stabilize floating nodes numerically.

Topology is compiled on edits. The native audio thread uses fixed-size storage and a nonblocking pending-state mailbox. Circuit replacements crossfade for 128 samples. Both outputs have a 12 Hz DC blocker and a hard safety clamp at ±1. This clamp can itself clip very hot output; lower Output to hear the circuit before the clamp. A failed solve holds the previous internal state and emits zero for that sample; the UI reports failures. Arbitrary invalid circuits are not guaranteed to converge.

Limits: 48 components, 32 total electrical nodes, 256 wires. Input/ground/battery cannot be directly shorted together. Generic silicon models, ideal passives, fixed temperature, no parasitic capacitances or realistic source impedance, no oversampling yet. This is a playable prototype, not a measured model of a specific vintage Fuzz Face or an engineering sign-off for a physical pedal.

## Validation and current limits

- C++ engine compiled and passed its analytical divider, RC time-constant, rewiring, short-rejection, transistor-bias-change, and finite-output/convergence checks on Linux.
- JavaScript solver passed corresponding numerical checks. The same 48 kHz / 110 Hz test produced matching bias and RMS results in C++ and JavaScript.
- The AudioWorklet processor passed a Node harness for demo audio, output bounds, stereo duplication, graph replacement, and dry bypass. This exercises processing logic, not a browser audio device.
- Native source is checked against pinned JUCE headers where possible; see VALIDATION.md for the final results.
- Mac AU/VST3 compilation, DAW loading/state recall, microphone permission flows, audible quality, browser rendering, and actual device latency still need testing on real devices. No Mac binary is included.

## What's next from the original README

1. Oversampling, improved DC initialization/solver fallback, parameter smoothing, measured transistor models, source impedance, and host/device validation.
2. Potentiometers, switches, PNP/germanium models, op amps, richer presets including Big Muff-inspired circuits.
3. Better native editor parity, node scopes, real-world tolerances, component automation and save-as-device.
4. Behavioral toy bend modules, BBD/PT2399 delay (web BBD demo exists; native plus PT2399 and algorithm-matched models are pending), modulation, and broader machine-readable schematic import.
5. Image/PDF schematic recognition, physical layout generation, and KiCad export.

The web app's bucket-brigade delay is a first-pass educational model — a fractionally interpolated line with a clock-derived darkening filter — not a measured MN3005/PT2399 or native-implemented build. The native plugin does not support BBD or pot/switch parts yet.

## Source map

- `dist/`: complete dependency-free web app; no build required.
- `Source/Circuit.h`: C++ graph compiler and circuit solver.
- `Source/Plugin.*`, `Source/Editor.cpp`: JUCE audio processor and native editor.
- `presets/`: shared circuit JSON and SPICE example.
- `tests/`: numerical and processor tests.
- `serve.py`, `run.sh`: local browser launcher.
- `build-native.sh`, `install-native.sh`, `CMakeLists.txt`: native build/install.

Tests: `node tests/web-engine.mjs` and `node tests/worklet-test.mjs`, or `cmake -S . -B build-core -DBREAD_ENGINE_ONLY=ON -DCMAKE_BUILD_TYPE=Release`, then `cmake --build build-core` and `ctest --test-dir build-core --output-on-failure`.

Framework documentation: https://github.com/juce-framework/JUCE/blob/8.0.6/docs/CMake%20API.md and https://juce.com/tutorials/tutorial_audio_processor_value_tree_state/ . JUCE is a third-party dependency with its own licensing terms; inspect JUCE's bundled license before distributing a product.
