# BREAD

**A real-time virtual electronics workbench where the circuit itself is
the audio effect.**

BREAD is an interactive audio-circuit environment for building,
rewiring, breaking, and experimenting with musical electronics while
sound is running.

Instead of loading a generic fuzz algorithm behind pictures of
components, BREAD models the actual circuit graph. Resistors,
capacitors, diodes, transistors, wires, bias changes, and topology edits
all affect the audio engine directly.

The project currently exists in two forms:

-   a dependency-free browser workbench for fast visual experimentation
-   a native JUCE project that builds as **VST3, AU, and standalone**

The long-term goal is to make electronic circuit design feel as
immediate and playable as a modular instrument: build a pedal, hear it,
change a component, reroute a wire, automate something physically
impossible, and eventually turn the result back into a buildable
hardware design.

## Why BREAD exists

Most audio plugins hide the electronics behind a finished interface.
Most circuit simulators are designed for engineering analysis rather
than live musical interaction.

BREAD sits between those worlds.

> **The breadboard is the instrument.**

A circuit should be editable while audio is running. A bad connection
should behave like a bad connection. Changing transistor gain should
change the sound. Pulling a wire should actually alter the graph. A
saved preset should be the circuit itself, not merely a list of abstract
DSP parameters.

That creates a tool that can function as an audio effect, an electronics
sandbox, a teaching environment, and eventually a bridge between
software experimentation and physical pedal building.

## Current prototype

The current milestone is centered around an editable two-transistor fuzz
circuit.

### Interactive workbench

The browser version currently supports:

-   drag-and-drop circuit components
-   explicit point-to-point wiring
-   live topology editing while audio is running
-   editable resistor, capacitor, diode, and NPN transistor values
-   loose / dangling patch leads when a connection is unplugged
-   component inspector with input validation
-   Undo, Reset, Save, and Open
-   device-local state persistence
-   JSON circuit import/export
-   partial SPICE netlist import
-   built-in demo audio
-   live input from a microphone or audio interface
-   local audio-file playback
-   oscilloscope output view
-   node-voltage probing / multimeter view
-   wet/dry, input, and output controls

The visual board does not hide electrical buses behind breadboard holes.
**Explicit wires define the electrical graph.**

### Native plugin

The native JUCE implementation builds as:

-   `BREAD.vst3`
-   `BREAD.component` (Audio Unit)
-   `BREAD.app` (standalone)

The native editor supports circuit editing, adding supported components,
wiring, value changes, JSON open/save, plugin-state recall, and host
automation for Input, Output, and Mix.

The browser version currently has the richer workbench UI; the native
editor is intentionally simpler while the DSP and state architecture
mature.

## The audio engine is actually solving the circuit

There is no hidden prewritten fuzz effect underneath the component
graphics.

Both the browser and native implementations solve the wire graph
electrically.

Current engine behavior includes:

-   conductance stamping for resistors
-   backward-Euler companion models for capacitors
-   exponential diode junction models
-   NPN transistor modeling using an Ebers-Moll-style approach with
    variable forward beta
-   a bounded, damped Newton iteration for nonlinear solving
-   pivoted elimination for the node-voltage system
-   topology compilation when the circuit changes
-   fixed 9 V supply and audio-input boundary nodes
-   numerical stabilization for floating nodes
-   DC blocking and output safety limiting
-   graceful handling of failed nonlinear solves

The current hard limits are 48 components, 32 electrical nodes, and 256
wires.

This is a playable prototype, not yet a precision model of a specific
vintage pedal or an engineering tool for hardware sign-off.

## Circuit formats

BREAD uses a shared graph representation so the UI, presets, native
plugin, browser engine, and import tools all describe the same
underlying circuit.

### JSON

Circuit state can be exported and reopened as JSON. That makes a saved
patch portable between implementations and gives future tooling a
structured format to target.

### SPICE subset

The browser workbench can currently import a practical subset of SPICE
including resistors, capacitors, diodes, NPN BJTs, potentiometers, and
selected `.model` information.

Unsupported devices are rejected rather than silently discarded.
Examples live in [`presets/`](./presets), including Fuzz Face-style and
RC filter netlists.

## Build and run

### Browser workbench

``` bash
bash run.sh
```

Requires Python 3 only. No package install, account, API token, Node
runtime, or virtual environment is required to run the app.

The local server opens the browser workbench at `http://localhost:8765`.

### Native VST3 / AU / standalone

Install Apple's command-line tools and CMake if needed:

``` bash
xcode-select --install
brew install cmake
```

Then:

``` bash
bash build-native.sh
```

The build pulls the pinned JUCE dependency and outputs native artifacts
beneath `build/Bread_artefacts/Release/`.

To install the plugin formats into the current user's plugin
directories:

``` bash
bash install-native.sh
```

For detailed implementation and validation notes, see
[`START-HERE.md`](./START-HERE.md) and
[`VALIDATION.md`](./VALIDATION.md).

## Validation

The project includes numerical and processor-level tests for both the
C++ and browser engines.

Current validation covers analytical resistor-divider behavior, RC time
constants, topology rewiring, short-circuit rejection, transistor bias
changes, finite-output/convergence behavior, C++/JavaScript solver
agreement, and AudioWorklet graph replacement and dry bypass.

Real-device validation of AU/VST3 behavior, DAW loading, hardware
latency, microphone flows, and subjective audio quality is still
ongoing.

## Project direction

Near-term development includes improved oversampling and solver
behavior, richer transistor/source models, potentiometers and switches
across both implementations, PNP/germanium support, op amps, more
presets, better native-editor parity, component automation/modulation,
and additional scopes and probes.

The broader direction includes BBD/PT2399-style delay systems,
behavioral circuit-bending modules, real-world component tolerances,
wider schematic import, image/PDF schematic recognition, BOM generation,
physical-layout export, and KiCad project generation.

The original, much larger product vision is preserved in
[`PROJECT_GOALS_README.md`](./PROJECT_GOALS_README.md).

## Repository map

``` text
Source/                 C++ circuit solver + JUCE plugin/editor
presets/                JSON and SPICE circuit examples
tests/                  numerical and processor tests
START-HERE.md           implementation notes and current milestone
VALIDATION.md           validation details
OSCILLATORS.md          oscillator design notes
TREMOLO.md              tremolo design notes
TOOLBOX.md              component/tooling notes
serve.py / run.sh       local browser launcher
build-native.sh         native build helper
install-native.sh       local plugin install helper
CMakeLists.txt          native/JUCE build configuration
```

## Status

**Active prototype / work in progress.**

BREAD already has a functioning circuit graph, real-time nonlinear
solver, browser workbench, native plugin target, shared circuit state,
import/export tooling, and test coverage. The next phase is focused on
fidelity, component coverage, UI parity, and expanding from the
two-transistor fuzz milestone into a more general-purpose musical
electronics environment.
