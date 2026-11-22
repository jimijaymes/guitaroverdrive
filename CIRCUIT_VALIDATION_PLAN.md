# Circuit-derived modelling and validation plan

Status: research and implementation specification; **not** a claim that these circuits are already hardware-calibrated.

## Core methodology

1. Select a **specific** original production revision and document its schematic, component values, control taper, signal path, supply and clipping stage. Distinguish original manufacturer schematics from traced clone schematics and blog interpretations.
2. Express each linear circuit block as a transfer function from its actual resistor/capacitor network. Include source/load impedances and potentiometer positions; don't assume isolated filters where they interact. Use nodal analysis or SPICE AC sweeps when required.
3. Fit a stable, oversampling-aware digital filter to the complex (magnitude **and phase**) response. Benchmark at 0%, 25%, 50%, 75%, 100% knob positions and at 44.1, 48, 96 kHz.
4. For nonlinear stages, retain topology: feedback diodes vs shunt diodes, transistor bias, op-amp slew/rails, interstage loading and headroom. Compare circuit transient/DC sweeps at multiple input amplitudes and frequencies.
5. Each **verified named modification** must be a patch list of component changes and the resulting recalculated model. Where only marketing descriptions are available, mark as approximation, not an exact mod.
6. The independent clipping control is an **override at the original circuit clipping node(s)**. In Follow Design, resolve clipping from the selected stock/mod variant (not always stock). A manual selection should not secretly add a second unrelated clipper. If a circuit has multiple clipping nodes, document which nodes the override affects; consider per-stage override later.
7. Add offline automated tests: frequency response, level, harmonic distortion, clipping symmetry, stability, bypass, parameter automation, presets, and latency. Avoid calling anything measured or validated until data is obtained.

## Inventory and reference coverage

| Model | Primary topology to verify | Starting reference | Specific validation risk | Priority |
|---|---|---|---|---|
| RAT | LM308 gain, gain-dependent HPF and compensation, shunt silicon clipping, reverse Filter pot | https://www.electrosmash.com/proco-rat | Slew approximation, filter pot/loading, Ruetz changes | High |
| Big Muff Triangle/Ram's Head | Four-transistor chain with two diode-feedback gain stages and passive scooped tone stack | https://electrosmash.mas-effects.com/big-muff-pi | Revision-specific components, transistor bias, tone stack loading | High |
| Tube Screamer TS9 | Feedback diode op-amp, gain-dependent HPF, active/passive tone, buffers | https://electrosmash.mas-effects.com/tube-screamer | Wet/dry blend assumptions, TS9 vs TS808 component differences | High |
| Boss SD-1 | Asymmetric feedback diode clipping, coupling and tone network | https://www.boss.info/ | Need an authoritative SD-1 schematic and version/component verification | High |
| DOD FX69 Grunge | Cascaded high-gain clipping and two-band EQ | Schematic and service data **not yet verified** | FX69 vs FX69B revision, gain/controls and pot taper | High |
| DOD FX86 Death Metal | Cascaded high-gain distortion and multiband EQ | Schematic and service data **not yet verified** | FX86 vs FX86B; do not confuse with FX56 American Metal | High |
| Boss BD-2 | Discrete multistage gain, frequency-dependent distortion and tone | https://www.cpearson.me.uk/2021/12/20/diy-boss-bd-2-blues-driver-keeley-mod/ | Not reducible to two generic feedback clippers; Keeley Phat C100 and diode changes | High |
| Boss DS-1 | Op-amp gain, hard diodes to ground, two-path tone network | https://electrosmash.mas-effects.com/boss-ds1-distortion | Pre/post production revision, exact Keeley Seeing Eye/Ultra diode + capacitor patch | High |
| Boss OD-2 | Normal and Turbo circuit paths | Manufacturer schematic/service notes **to verify** | Distinct switching networks, not a gain multiplier | High |
| Boss HM-2 | Distortion cascade and three gyrator resonances, two high resonances linked | https://aionfx.com/app/files/docs/graviton_documentation.pdf ; https://bench.swellamps.com/filters/pedal-hm2/ | Correct interactive EQ and pot taper, Q/loading, diode stages | High |
| Boss SD-2 | Separate Crunch and Lead circuitry | https://www.synfo.nl/servicemanuals/Boss/BOSS_SD-2_SERVICE_NOTES.pdf | True dual path and mode switch topology | High |

## Named mod acceptance rules

- **DS-1**: Keeley Seeing Eye / Ultra: use documented D4/D5 diode arrangement, additional caps/resistors and any switch-dependent network. The existing partial implementation is not yet exact.
- **BD-2**: Phat: switch actual C100 network and other documented changes; validate whether each variant changes clipping devices or only EQ.
- **HM-2**: stock gyrators; Dunwich low-tuned EQ requires exact frequency/Q/gain changes from component substitutions. Generic bass/presence modes must not be labelled as established boutique mods.
- **RAT, Big Muff, TS9, SD-1**: translate previously researched named mod variants into component-change manifests; reject undocumented attribution.
- **DOD FX69/FX86, Boss OD-2/SD-2**: do not invent branded mod variants without public schematic evidence. Stock first.

## Development milestones

1. **Reference dataset**: revision, schematic URL, parts table, signal-flow graph, control tapers, source confidence, and known named mod patches for all 11.
2. **Linear stage correction**: tone networks and frequency-dependent gain for RAT, TS9, DS-1, Big Muff and HM-2.
3. **Complex distortion correction**: BD-2, OD-2, SD-2 and DOD FX69/FX86.
4. **Nonlinear fitting**: diode/transistor stage solving and real circuit headroom, using SPICE as reference.
5. **Regression and audio validation**: automated sweeps, reference recordings when available, Windows VST3 CI and listening tests.

## Engineering caveats

A public schematic establishes a nominal circuit, not exact real-unit performance. Component tolerances, diode I–V curves, device substitutions and guitar/source loading affect the result. A YouTube audio demonstration alone is unsuitable as a calibration target due to unknown input, amp, cab, mic and recording chain. Never reuse proprietary plugin source code without permission.

## Initial sources

- https://electrosmash.mas-effects.com/ (archive of analyses for RAT, Big Muff, Tube Screamer, DS-1, MXR)
- https://www.electrosmash.com/proco-rat (component-level RAT analysis)
- https://bench.swellamps.com/filters/pedal-hm2/ (HM-2 EQ simulation)
- https://aionfx.com/app/files/docs/graviton_documentation.pdf (HM-2 clone circuit)
- https://www.synfo.nl/servicemanuals/Boss/BOSS_SD-2_SERVICE_NOTES.pdf (SD-2 service notes)
- https://www.cpearson.me.uk/2021/12/20/diy-boss-bd-2-blues-driver-keeley-mod/ (BD-2 modification walkthrough)
