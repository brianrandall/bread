# LFO tremolo

Choose **Circuit Collection → LFO tremolo → Load preset**, then start a test oscillator, demo, live input or audio file.

Select **LFO1** on the board to choose sine, square or triangle, speed (0.1–20 Hz), and depth (0–100%).

The preset patches audio into VCA1, LFO1.out into VCA1.cv, and VCA1.out through the PV volume pot to output. Both modules have ground connections. Unplug the CV wire to close the VCA. At 0% depth, the LFO holds 1 V for steady full gain. Wet/dry remains the circuit blend.

LFO and VCA are reusable parts in the drawer. Each LFO has its own speed, waveform and depth, saved with the circuit. VCA maximum gain is editable from 0–10; its CV range is 0–1 V. These are behavioral modules, with no LED or LDR. They do not model a particular chip or battery starvation. Square edges have 1 ms smoothing.

The previous global tremolo panel and post-output effect have been removed.

Validation: run `node tests/tremolo-test.mjs`.
