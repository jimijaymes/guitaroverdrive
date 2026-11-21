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
The Mod menu is contextual to the selected pedal. Mod Amount, Mod Tone and Mod Bass are additional controls; only some mods use each one. Mod switches are exclusive (one selected mod at a time), not stackable.

| Model | Mod 1 | Mod 2 | Mod 3 | Mod 4 |
| --- | --- | --- | --- | --- |
| RAT | Ruetz-style gain/bass-response change (Amount) | Turbo LED clipping | Fat pre-clipping response (Amount) | Parallel clean blend (Amount) |
| Big Muff | Tone-stack bypass | Mid control (Amount) | Creamier/bassier response (Mod Bass) | Flatter midrange |
| Tube Screamer | Bass+ feedback high-pass shift (Amount) | TS808-inspired mix/response | Keeley-style extended bass and clipping options (Amount, Mod Bass) | Alternative clipping thresholds (Amount) |
| SD-1 | C6-inspired high-frequency shift (Amount) | Bass response mod (Amount) | Keeley-style extended bass/clipping (Amount, Mod Bass) | Alternative clipping thresholds (Amount) |

The clipping menu also allows stock, LED, germanium, asymmetric, MOSFET-inspired and no-diode behaviour. RAT Turbo overrides the diode menu while active. Stock SD-1 uses asymmetric feedback clipping; stock Tube Screamer uses symmetric feedback clipping; stock RAT uses hard-to-ground silicon diode clipping. The Big Muff uses two clipping stages and a bass/treble tone stack.

These are **inspired by** common modifications, not verified exact resistor/capacitor substitution simulations. In particular, TS808 and Keeley-style modes need calibration against reference hardware, and MOSFET clipping is a threshold approximation.

## References
- ElectroSmash RAT analysis: https://www.electrosmash.com/proco-rat
- ElectroSmash Tube Screamer analysis: https://www.electrosmash.com/tube-screamer-analysis/pedals/distortion/tube-screamer-analysis.html
- RAT modifications overview: https://www.sabrotone.com/proco-rat-w-lots-of-options/

## Build
GitHub Actions Windows runner builds JUCE 8.0.10 VST3 and Standalone using CMake/Visual Studio 2022. A successful CI run and in-host audio test are required before treating this as release-ready.

## DSP maintenance
Saturator is the reference implementation for reusable transformer algorithms. When Saturator transformer modelling changes, explicitly review this repository for an applicable port. Repositories are not automatically synchronized.
