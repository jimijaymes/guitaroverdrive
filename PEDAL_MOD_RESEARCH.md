# Pedal modification research and implementation backlog
Research begun 2026-11-22. **Research only:** these are not yet DSP implementations. Verify exact schematic revisions and component changes before claiming an authentic boutique mod.

| Priority | Pedal | Mod / reference | Proposed circuit work | Evidence |
|---|---|---|---|---|
| 1 | TS9 | Monte Allums Tri-Gain | 1N914 / LED / diode lift, gain and presence options | https://monteallums.com/pedal_mods.html |
| 1 | TS9/808 | Analog Man Silver | Research less-coloured voicing, buffers and feedback/output networks; proprietary details unverified | https://www.analogman.com/ts9silv.htm |
| 1 | TS9DX | Analog Man Mode Mods | + bass, HOT asymmetrical clipping, TURBO extended bass; separate RC and diode topology | https://www.analogman.com/ts9dx.htm |
| 1 | SD-1 | Wampler SD-1-to-TS808 conversion | Documented resistor/capacitor/diode/gain pot changes; use component-derived model | https://www.youtube.com/watch?v=kWTrXkQS-50 |
| 1 | SD-1 | Monte Allums SD-808 | TS808-like circuit and symmetric clipping; beyond global diode selection | https://www.monteallums.com/pedal_mods_SD1.html |
| 1 | SD-1 | Monte Allums GT / Stacked Plus | Investigate 2N3440 transistor, presence, gain and response; exact component values unverified | https://monteallums.com/pedal_mods.html |
| 1 | RAT | Keeley / Mighty Mouse | Research diode switch and capacitor/gain-filter changes; published community accounts conflict on exact clipping options | https://www.reddit.com/r/guitarpedals/comments/1rn5g0k/keeley_rat_mod_any_details/ |
| 1 | RAT | Monte Allums FAT RAT | Research coupling/bass and clipping revisions, exact network unverified | https://monteallums.com/wordpress/product/pedal-mod-pdf-instructions-list-2/ |
| 1 | RAT | Aion FX Helios / Heliosphere | Baseline RAT network, clipping switching and gain presets | https://aionfx.com/project-type/pcb/ |
| 1 | Big Muff | Aion FX Halo | Mid scoop / flat / hump switch; authentic passive tone-stack and vintage component sets | https://aionfx.com/project/halo-distortion-sustainer/ |
| 1 | Big Muff | Aion FX Halo legacy | Presence/mids, optional diode arrangements, JFET gain recovery | https://aionfx.com/project/halo-legacy/ |
| 2 | Big Muff | Cornish P-1/P-2/G-2 and Aion FX Cygnus | Input buffers, loading, frequency response, component-level differences | https://aionfx.com/app/files/2019/03/cygnus_documentation_v1.pdf |
| 2 | Big Muff | Op-Amp Big Muff / Aion FX Corvus | Distinct op-amp architecture, not merely a clipping option | https://aionfx.com/project-type/pcb/ |
| 2 | Boss DS-1 | Keeley / Analog Man / Monte Allums; Aion FX Comet | New DS-1 model with pre/post-1994 revisions, hard clipping and tone stack | https://aionfx.com/project/comet-legacy/ |
| 2 | Boss BD-2 | Keeley Phat and other documented modifications | New BD-2 architecture, verify circuit modifications | https://aionfx.com/build/ |
| 3 | Other circuits | Boss OD-3, Nobels ODR-1, Klon, Bluesbreaker, OCD, DOD 250, Fuzz Face | Separate models where topology genuinely differs | https://aionfx.com/projects/ |

## Implementation standards
1. Verify pedal revision, schematic, part substitutions, bias and loading.
2. Model actual RC corner frequencies, gain distribution and diode location; don't fake named mods with generic EQ presets.
3. Keep mods independently stackable only where physically meaningful, with existing global diode selector independent.
4. Avoid breaking preset parameter IDs; distinguish stock, research, implemented and validated states.
5. Require CI pass and DAW/audio checks before describing anything as production-ready.
6. Aion FX is primarily a schematic/project-documentation reference, not automatically the originator of every historical modification. Wampler, Keeley, Analog Man and Monte Allums modifications should be attributed accurately.

## Expansion phase — Boss, Electro-Harmonix, MXR and DOD (research only)

**Scope:** add distinct circuit families only where their signal flow justifies a new model. Existing RAT/Muff/TS/SD-1 models and mods remain unchanged. Prioritize gain/fuzz/boost first; compressors, EQs and modulation need dedicated DSP and should not be misrepresented as diode mods.

### Boss
| Priority | Model | Named modification / variants to research | DSP requirements | Source / confidence |
|---|---|---|---|---|
| High | DS-1 | Keeley Seeing Eye / Ultra; Monte Allums Rectifier Tri-Gain | Distortion op-amp gain, shunt clipping and tone-stack; diode combinations, low-end RC changes | Monte Allums documented catalogue: https://monteallums.com/wordpress/product/pedal-mod-pdf-instructions-list-1/ ; Keeley exact revisions need verification |
| High | BD-2 | Keeley Phat; Monte Allums Blues Stack, BD-H2O | Cascaded gain, dynamic clipping and bass response; Phat switching | https://monteallums.com/wordpress/product/pedal-mod-pdf-instructions-list-1/ ; proprietary circuit details require verification |
| High | MT-2 | Monte Allums Sustainia / Tri-Gain; stock MT-2 | Multiple clipping/gain stages and complex active mid EQ; optional germanium and flatter EQ | https://monteallums.com/wordpress/product/mt-2-sustainia-mod/ |
| High | HM-2 | Monte Allums Vintage Plus; stock HM-2 | Distortion + distinctive dual-peak EQ, independent from MT-2 | https://monteallums.com/wordpress/product/pedal-mod-pdf-instructions-list-1/ |
| Medium | OD-3 | Monte Allums mods and stock circuit | Separate overdrive topology, dynamic response and tone control | https://monteallums.com/Pedal_Settings.html |
| Medium | DS-2 | Turbo mode variations and documented boutique mods | Distinct mode-dependent gain/EQ networks; validate actual mod documentation | research pending |
| Medium | CS-3 | Monte Allums Opto Plus / Ultra Plus / Noise Decimator | Compression envelope/attack, noise floor and bass switching; **not a distortion mode** | https://www.monteallums.com/pedal_mods_CS3.html |
| Medium | GE-7 | Monte Allums low-noise Plus / vintage revisions | Seven graphic EQ bands, op-amp noise and headroom; model 3- vs 4-op-amp revision only with verification | https://monteallums.com/wordpress/product/boss-ge-7-vintage-ge-7-geb-7-mod-kits-regular-kits/ |

### Electro-Harmonix
| Priority | Model | Historically significant variants/mods to investigate | DSP requirements | Research status |
|---|---|---|---|---|
| High | Big Muff Pi | Green Russian, Civil War, Black Russian, NYC, Triangle, Ram's Head, Op-Amp; tone-bypass and mids mods | Separate transistor/op-amp architecture, RC/gain, clipping stages and passive tone stack | Product families established; verify revision-by-revision component maps |
| High | Hot Tubes | Vintage CMOS and reissue differences, documented bass/gain mods | CMOS transfer characteristic, frequency-selective drive | Exact boutique mods unverified |
| High | Double Muff | Dual Muff Fuzz cascade and bypass/stack options | Two cascaded clipping circuits, gain staging | Verify circuit variants |
| Medium | LPB-1 | Vintage/reissue transistor boost and documented bass/treble coupling mods | Single-transistor boost, bias/loading and frequency response | Verify component values |
| Medium | Soul Food | Klon-family circuit; diode and bass modifications | Separate clean/mid-gain paths and mixing | Verify mod provenance |
| Medium | Small Clone / Electric Mistress | Depth/vibrato or clock/noise modifications | BBD chorus/flanger, modulation and anti-alias filtering; separate future plugin family | Do not force into overdrive engine |
| Medium | Memory Man | Headroom, clock, preamp and modulation modifications | BBD delay + nonlinear preamp; separate future plugin family | Verify revision and modification documentation |

### MXR / DOD
| Priority | Model | Mods / variant research | DSP requirements | Research status |
|---|---|---|---|---|
| High | MXR Distortion+ | Germanium vs silicon/LED clipping, gain-capacitor and bass response mods | Op-amp with shunt diodes, no RAT-style filter topology | Historical family clear; boutique attribution needs sourcing |
| High | DOD Overdrive Preamp 250 | Grey/yellow revision, clipping diode and bass-cap mods; boutique 250 variants | Op-amp gain and shunt clipping with revision-dependent filtering | Verify component values and revision history |
| High | MXR Micro Amp | Gain range, output response, optional tone/bass mods | Clean gain, frequency response, op-amp rail/headroom | Verify actual mods |
| Medium | MXR Dyna Comp | Ross conversion, attack/release and blend mods | OTA compressor and envelope detector, not generic clipping | Verify mod attribution |
| Medium | MXR Blue Box | Octave tracking, output/blend and gating mods | Rectification/divide-down suboctave and fuzz | Verify revisions |
| Medium | DOD FX52 Classic Fuzz | Diode/gain and bias mods | Fuzz-specific transistor clipping and filtering | Documented mod source pending |
| Medium | DOD FX56 American Metal / FX69 Grunge | Clipping and tone-network mods | Distinct distortion architecture, avoid generic diode-only presets | Documented mod source pending |

### Modelling implementation order
1. DS-1 with its own clipping/tone network and Keeley/Monte Allums-inspired options once exact changes are verified.
2. BD-2 and MT-2 (distinct topologies), then HM-2.
3. Big Muff additional verified vintage revisions and separate Op-Amp Muff.
4. MXR Distortion+ and DOD 250, then LPB-1 and Micro Amp.
5. Compressors/EQ and modulation/delay only as separate DSP modules or plugins.

**Verification gate:** a product listing or anecdote establishes that a named mod exists, not its precise schematic. Do not label an approximate implementation as an exact Keeley, Wampler, Analog Man, Monte Allums or Aion FX circuit without schematic or measurement evidence.

## Confirmed next seven dedicated pedal models — requested 2026-11-22

These are **approved development targets, not yet implemented DSP**. Keep RAT, Big Muff, TS9, SD-1, boost models and existing parameter IDs intact. Each new pedal requires its own topology and independently selectable model entry; do not simulate these by relabelling an existing distortion circuit.

| Model | Required architecture / behaviour | Variants / mod research | Acceptance criteria |
|---|---|---|---|
| DOD FX69 Grunge | Original distortion stages, gain and distinctive dual-band LOUD/BUTT/FACE-style control behavior (verify exact revision and control labels) | FX69 vs FX69B revisions, documented component/diode modifications | Level-match stock, independently tune low and high shaping, verify clipping location |
| DOD FX56 American Metal ("DOD Heavy Metal") | Distinct high-gain clipping and active/passive tone network per verified FX56 schematic | Revision history and documented mods | Do not conflate with Boss HM-2 or DOD FX69 |
| Boss BD-2 Blues Driver | Cascaded gain with nonlinear response and interactive frequency shaping; model its dynamic behavior | Keeley Phat, Monte Allums Blues Stack/H2O where component changes are documented | Low-drive touch response, guitar-volume cleanup and bass switch behavior |
| Boss DS-1 Distortion | Distortion amplifier, diode-to-ground clipping and dedicated tone network | Keeley Seeing Eye/Ultra, Monte Allums Rectifier Tri-Gain; verify pre/post-1994 differences | Independent diode network and correct tone-stack transfer |
| Boss OD-2 Turbo OverDrive | Distinct Normal and Turbo circuit paths with separate gain/headroom/voicing | OD-2 vs OD-2R revision, documented modifications | Turbo is not just a gain multiplier |
| Boss HM-2 Heavy Metal | Cascaded distortion and highly characteristic resonant Low/High tone network | Original HM-2 vs HM-2W modes; Monte Allums Vintage Plus if verified | Reproduce multi-peak EQ instead of a generic bass/treble shelf |
| Boss SD-2 Dual OverDrive | Independent Crunch and Lead circuit models with switchable mode | SD-2 mode switching, documented modifications | Crunch and Lead have genuinely different nonlinear architecture |

### Integration plan
- Expand the pedal choice by **appending** seven entries to preserve existing saved indices: DOD Grunge, DOD Heavy Metal (FX56), Boss BD-2, Boss DS-1, Boss OD-2, Boss HM-2, Boss SD-2.
- Add a model-specific mode selector for OD-2 Normal/Turbo and SD-2 Crunch/Lead. Only show/enable relevant controls.
- Existing pedal-specific mods must not leak into other models. Add new mod controls after validating schematic-level changes.
- Keep global input/output HP/LP, transformer, oversampling, dry/wet mix, and TC/FA-1 pre/post boost available for every model, including distortion bypass.
- Research schematics, revision differences, clipping topology, RC filtering, tone stack and output level before writing DSP. Benchmark at several drive/tone settings and verify builds.

**Naming note:** DOD Heavy Metal is taken to mean the **FX56 American Metal** in the absence of a specific model number; confirm before implementing if another DOD model was intended.
