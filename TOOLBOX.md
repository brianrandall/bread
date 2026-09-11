# BREAD — toolbox pass

Web app additions: three-terminal pots (10 Ω–10 MΩ), linear/log taper, live wiper control, SPST switches, and selectable silicon/germanium/Schottky/red-LED junction approximations.

Choose a circuit in Circuit Collection, then Load preset. Undo restores the previous board. Includes the original fuzz, Volume pot lab, Switchable low-pass, and Diode clipping lab. Select a component to use its inspector controls. Increase Input for audible clipping in the passive diode lab.

Pots have track endpoints 1/2 and wiper w; 0% touches endpoint 1. Log taper uses (100^position - 1)/99. Track segments have a 1 Ω minimum for numerical stability. Closed switches use 1 Ω contact resistance; open switches stamp no conductive branch. Existing numerical floating-node stabilization still applies.

Diode families change saturation current and ideality, not just labels. They are simplified educational approximations, without measured device matching, junction capacitance, breakdown, or temperature behavior. Custom saturation-current mode remains available.

All part settings survive JSON save/open and browser-local recall. These additions are web-only: the native plugin still supports the original R/C/NPN/custom-diode graph and does not yet support pots/switches or named diode models. Do not expect new web circuits to sound equivalent in the native prototype.

Validation: existing engine/AudioWorklet checks plus tests/toolbox-test.mjs cover pot endpoints and taper, open/closed switch behavior, diode-family ordering, preset convergence and JSON round trips. Browser rendering and real-device listening were not tested here. No oversampling was added in this pass.
