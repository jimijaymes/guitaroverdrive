# Guitar Overdrive — user manual (initial DSP build)

Four switchable pedal-inspired circuits: RAT, Big Muff, Tube Screamer, Boss SD-1. The models are topology-informed digital approximations, not verified circuit-identical recreations. Default pedal is RAT. Stock mod and Stock clipping are defaults.

## Common controls
- Input: gain before pedal stages.
- Distortion: circuit-specific gain, not an interchangeable clipper drive.
- Tone / Filter: RAT clockwise darkens; other pedals clockwise brightens.
- Level: final pedal level before optional transformer and output filters.
- Mix: latency-aligned dry/wet.
- Input HP/LP: additional adjustable filtering before pedal.
- Output HP/LP: additional adjustable filtering after pedal and transformer.
- Transformer: Off, Neve 10468 input, Neve LO1166 output, API 2622 + 2520, API 2520 + 2503, 1176 UTC input, 1176 Class A output, 1176 Rev F output. Based on Saturator's leaky-flux memory, amplifier and magnetic distortion branches, model-specific frequency response. Transformer Drive adjusts only this stage.
- Oversampling: 1×/2×/4×/8×. Fixed maximum latency reported to host, with dry and wet paths aligned. Oversampling quality, alias rejection and latency must be checked in a compiled plugin.

## Pedal-specific modifications
Four contextual mod switches are independent and can be stacked. Mod Amount, Mod Tone and Mod Bass are additional controls; only some mods use each one. The clipping selector is independent and available on all four pedals. Clipping types themselves are mutually exclusive, as they represent alternative diode configurations. Some combined modifications interact, rather than simply adding their effects.

| Model | Mod 1 | Mod 2 | Mod 3 | Mod 4 |
| --- | --- | --- | --- | --- |
| RAT | Ruetz-style gain/bass-response change (Amount) | Gain EQ / filter (reserved) | Fat pre-clipping response (Amount) | Parallel clean blend (Amount) |
| Big Muff | Tone-stack bypass | Mid control (Amount) | Creamier/bassier response (Mod Bass) | Flatter midrange |
| Tube Screamer | Bass+ feedback high-pass shift (Amount) | TS808-inspired mix/response | Extended bass (Mod Bass) | Bright tone (Mod Tone) |
| SD-1 | C6-inspired high-frequency shift (Amount) | Bass response mod (Amount) | Extended bass (Mod Bass) | Bright tone (Mod Tone) |

The clipping menu also allows stock, LED, germanium, asymmetric, MOSFET-inspired and no-diode behaviour. RAT Turbo is selected through the global LED clipping option, rather than an exclusive pedal mod. Stock SD-1 uses asymmetric feedback clipping; stock Tube Screamer uses symmetric feedback clipping; stock RAT uses hard-to-ground silicon diode clipping. The Big Muff uses two clipping stages and a bass/treble tone stack.

These are **inspired by** common modifications, not verified exact resistor/capacitor substitution simulations. In particular, TS808 and Keeley-style modes need calibration against reference hardware, and MOSFET clipping is a threshold approximation.

## References
- ElectroSmash RAT analysis: https://www.electrosmash.com/proco-rat
- ElectroSmash Tube Screamer analysis: https://www.electrosmash.com/tube-screamer-analysis/pedals/distortion/tube-screamer-analysis.html
- RAT modifications overview: https://www.sabrotone.com/proco-rat-w-lots-of-options/

## Build
GitHub Actions Windows runner builds JUCE 8.0.10 VST3 and Standalone using CMake/Visual Studio 2022. A successful CI run and in-host audio test are required before treating this as release-ready.

## DSP maintenance
Saturator is the reference implementation for reusable transformer algorithms. When Saturator transformer modelling changes, explicitly review this repository for an applicable port. Repositories are not automatically synchronized.

## Stacking implementation note
The four switches are independent APVTS boolean parameters, and the diode selection is always separate. RAT Mod 2 is reserved and currently has no DSP effect; it must not be advertised as an active modification until a distinct non-clipping circuit change is implemented. Clipping alternatives cannot be stacked with each other as a single diode network; choose one configuration per pedal. The current UI changes have not yet been validated in a host.

## Early Big Muff variants
The Big Muff Version selector offers Triangle and Ram's Head. Both used silicon diode pairs in transistor clipping stages; neither is intrinsically a germanium model. The Stock clipping setting uses different nominal silicon conduction and stage loading approximations; global diode selection overrides both clipping stages. Triangle and Ram's Head have separately tuned booster gain, interstage gain, coupling bass corner, high-frequency rolloff and tone-stack crossover. Actual units vary considerably by production revision, and these are initial approximations, not verified schematic-specific netlists. Independent tone mods and diode selection remain stackable.

### Early Muff clipping refinement
Stock Triangle and Ram's Head use separate silicon-pair conduction curves with different effective onset and softness for their two clipping transistor stages. These curves are circuit-inspired approximations of diode conduction plus stage loading, not claims that every vintage production unit used a particular diode part. Schottky is offered as a separate optional clipping selection, never the default for either early model. Modern NYC Big Muff variants are not assumed to use Schottky diodes universally; component populations vary. Calibration against specific schematics and hardware remains outstanding.

## Clipping diode selector (independent of mods)
The global clipping selector has **Follow Design** as its default. This preserves each pedal's model-specific clipping: RAT stock silicon-to-ground, Big Muff Triangle/Ram's Head distinct vintage silicon transistor-feedback clipping curves in both stages, Tube Screamer symmetric silicon feedback clipping, and SD-1 asymmetric silicon feedback clipping. The selectable overrides are LED pair, germanium pair, asymmetric silicon, MOSFET-inspired, no diodes, Schottky pair, symmetrical Si+LED series strings, low-forward silicon pair and high-forward silicon pair. The **Si + LED Series (Symmetric)** option models one silicon diode and one LED in series in each opposing polarity (nominal combined threshold ~2.35 V in the simplified clipper). It does not mean one silicon diode on the positive side and one LED on the negative side, which would be asymmetric. These are phenomenological clipper approximations, not SPICE models; thresholds are nominal and should be calibrated to hardware. Existing sessions with saved clipping indices 0–6 retain the same approximate sound, although labels are more descriptive.

## Pedal-specific clipping topology
Diode choices select a device configuration **within the chosen circuit**, not a universal post-distortion waveshaper. RAT uses the selected diode pair as a hard shunt-to-ground clipper after its slew-limited op-amp. Triangle and Ram's Head Big Muffs apply overrides independently in two transistor feedback-clipping stages, with stage-specific drive and feedback strength; Follow Design retains their separate early-silicon approximations. Tube Screamer and SD-1 use a nonlinear feedback-network approximation rather than a post-gain hard clip; the SD-1 Follow Design choice remains asymmetric. The solver is a stable static approximation, not a component-by-component dynamic op-amp/transistor SPICE simulation. Frequency-dependent feedback, diode junction capacitance and device recovery remain future calibration work.

## TS9 / SD-1 early HPF capacitor
The three-position TS9 / SD-1 HPF Cap selector changes the early feedback high-pass capacitance relative to the stock model: Stock (1x), Larger (1.5x), Smaller (0.68x). With the current 720 Hz stock approximation and other bass mods off, the corresponding corner frequencies are 720 Hz, 480 Hz and approximately 1059 Hz. At fixed resistance, cutoff is inversely proportional to capacitance. The setting is enabled only for Tube Screamer and SD-1 and remains independent of clipping and stackable bass mods. Exact production capacitor values are not yet verified against a chosen schematic revision.

## TC Integrated Preamp-style boost
Independent boost stage, with Off (default), Pre Pedal and Post Pedal routing. Gain range 0 to +24 dB, default +12 dB. Pre routing places the boost after the global input HP/LP and before the selected distortion pedal; Post routing places it after the distortion pedal but before the optional transformer and global output HP/LP. All four pedal models, diode configurations, HPF capacitor options and pedal-specific mods remain available. The boost has its own per-channel coupling HPF (~28 Hz), gentle HF rolloff (~16 kHz) and mild rail-limited amplifier saturation, with independent pre/post filter state. It is a TC Integrated Preamp-inspired starting approximation, **not** a schematic-verified TC Electronic Integrated Preamp reproduction; the original hardware's bass/treble controls, gain structure and loading are not yet explicitly modelled. Post boost may drive the transformer stage into additional saturation. Boost remains inside the plugin wet path, so the global Mix control blends the entire boosted chain with dry.

## TC / Boss FA-1 boost and pedal bypass
Boost Circuit switches between TC Integrated Preamp-inspired and Boss FA-1-inspired gain/response models. They have separate approximate input coupling, HF rolloff and amplifier rail saturation characteristics; neither is a verified transistor-level reconstruction of the original circuit. Boost Routing remains Off / Pre Pedal / Post Pedal and Boost dB applies to both models. **Bypass Distortion Pedal** skips the entire RAT/Muff/TS/SD-1 processing block (including its output gain) while retaining the boost, transformer and global input/output filters, enabling boost-only use. The boost has an optional **MID EQ ON** switch and semi-parametric Mid Frequency (180–7200 Hz) and Mid Gain (-15 to +15 dB) controls. A broad proportional-Q state-variable bell is inspired by the Neve 1073 mid-band response, but is not a component-accurate Neve 1073 inductor EQ; the 1073 hardware uses switched frequencies rather than continuously variable frequency. This EQ is in the boost circuit and works for either boost model, pre or post, including when distortion is bypassed. Off boost routing bypasses the entire boost and its mid EQ. All new options default to off, preserving previous project behaviour.

## Seven additional pedal algorithms — initial DSP pass

The pedal menu now appends DOD FX69 Grunge, DOD FX86 Death Metal, Boss BD-2, Boss DS-1, Boss OD-2, Boss HM-2 and Boss SD-2 after the original four entries (existing indices remain unchanged). Each uses a dedicated processing branch with model-dependent gain stages, filtering and clipping rather than an existing-pedal preset. OD-2 Normal/Turbo and SD-2 Crunch/Lead use the new OD-2 / SD-2 Mode selector; it is disabled for other models. Pedal Bass and Pedal Treble affect the new distortion branches, especially DOD FX69, FX86, DS-1 and HM-2. The existing global Tone knob also remains available where implemented.

The four independent mod toggles on the new families currently provide bass coupling, presence, clipping density and clean blend, respectively. These are **circuit-inspired development options, not verified named boutique modifications**. Their names should not be marketed as Keeley, Wampler, Analog Man, Monte Allums or Aion FX modifications without a verified component-level implementation.

**Accuracy limits:** All seven algorithms are first-pass topology-inspired approximations. In particular, HM-2's resonant EQ is not yet an exact dual-resonance filter model; FX86 gain and EQ require schematic verification; and OD-2/SD-2 mode-specific component networks require further research. Stock selections mean stock-*inspired* starting voicings, not hardware-calibrated or component-accurate clones. Do not claim completed hardware emulation or audio validation until reference measurements and listening tests are performed. Oversampling, transformer, optional boost and pedal bypass remain in the shared processing path.
