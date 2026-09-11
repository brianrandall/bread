# Two test oscillators

The browser workbench now has a **Test input** panel below the signal-chain controls and scope. Press **Test oscillators** to send both sources through the circuit.

Each oscillator has sine, square, triangle, or noise; independent volume and an Enabled switch; a logarithmic tuning slider; exact frequency entry from 20 to 12,000 Hz; octave buttons; and a musical note/cents readout. Frequency changes, volume changes, mutes, and waveform crossfades use short smoothing ramps.

For noise, frequency sets the low-pass cutoff rather than a musical pitch. Each source has its own four-second loop of independently generated white noise. Lowering the cutoff darkens it.

Try:

- A = 110 Hz, B = 220 Hz: octave.
- A = 110 Hz, B = 165 Hz: just fifth.
- A = 110 Hz, B = 111 Hz: slow beating.
- Mute B and compare sine, square, and triangle through the fuzz.
- Set Wet/Dry to 0% to hear the clean test source; raise it to hear the circuit.

The two voices are summed before BREAD's main Input control. Each is scaled to a maximum gain of 0.25 to leave mixing headroom; use Input to drive the circuit harder. Test oscillators, live input, audio-file playback, and the demo are alternative sources. Switching sources stops and disconnects the old source, including microphone tracks. Stop shuts the oscillators down completely.

Settings are remembered in this browser. They are separate from circuit JSON. This update adds the test panel to the web app; the native plugin's source is unchanged.

Checks: JavaScript syntax, existing circuit/audio processor tests, and `node tests/tones-test.mjs` for tuning, independent volume/mute, noise filtering controls, summing connections, and source cleanup. The new test uses a mock Web Audio context; it does not verify browser rendering or audio output on a physical device.

Implementation references: [OscillatorNode](https://developer.mozilla.org/en-US/docs/Web/API/OscillatorNode), [AudioParam smoothing](https://developer.mozilla.org/en-US/docs/Web/API/AudioParam/setTargetAtTime), [BiquadFilterNode](https://developer.mozilla.org/en-US/docs/Web/API/BiquadFilterNode).
