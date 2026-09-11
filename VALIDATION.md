# Validation — 2026-09-08

## Passed in the build environment (Linux)

- CMake 4.4.3 engine-only configure and Release build using GCC 13.3.
- CTest: circuit_engine, 1/1 passed.
- C++ JUCE processor and editor: `g++ -std=c++17 -fsyntax-only` using the pinned JUCE 8.0.6 headers (commit 51a8a6d7aeae7326956d747737ccf1575e61e209). This checks compilation syntax/API usage, not linking or plugin wrapper generation.
- JavaScript engine tests under Node 24: divider, RC step response, supply-short rejection, disconnected input equivalence, invalid-value rejection, strict SPICE import, state restore, fuzz finite output and convergence.
- AudioWorklet Node harness: actual processor class, one second of generated demo input, finite bounded output, equal L/R output, circuit state replacement, dry bypass.
- JavaScript syntax checks, shell syntax checks, Python compile check, local static entrypoint/asset-reference checks.

Matching model results at 48 kHz after 1024 silent settling samples:

| Measurement | C++ | JavaScript |
| --- | ---: | ---: |
| Q1 collector voltage | 1.48702 V | 1.48701754 V |
| Q2 collector voltage | 0.853276 V | 0.85327565 V |
| Raw output RMS, 0.06 V / 110 Hz sine, 48,000 samples | 2.15216 V | 2.15216265 V |
| Failed sample solves in this test | 0 | 0 |

These are results of the same simplified equations, not comparison with hardware or a SPICE reference. The preset's Q2 is heavily biased toward saturation. Fidelity and useful bias ranges need listening and hardware-reference work.

## Not verified here

- A complete linked plugin or standalone build. This environment lacks JUCE's Linux GUI/audio system development dependencies; package installation was unavailable. No compiled plugin is delivered.
- macOS arm64 builds, AU validation, DAW scanning, automation, and session save/restore in a host.
- Browser rendering, pointer/keyboard workflows, getUserMedia prompts, real audio device I/O, or end-to-end latency. No browser/visual testing was requested, so no browser QA was performed.
- Perceptual sound quality, aliasing performance, or correspondence with a vintage pedal.

The native project and Mac scripts are provided for building and testing on the user's M1 Pro. Passing numerical tests is not a claim that the complete README roadmap is implemented.
