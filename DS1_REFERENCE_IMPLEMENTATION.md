# DS-1A circuit-derived first implementation

Source: https://www.electrosmash.com/boss-ds1-analysis (modern DS-1A schematic; **not** original 1978 TA7136AP version).

## Current DSP path
1. Input high-pass 7.2 Hz (C1 network)
2. Interstage high-passes 3.3 and 33 Hz (C2/R4, C3/R5)
3. Q2 shunt-feedback transistor booster: nominal 56x small-signal gain, asymmetric soft supply limiting around +4/-3 V; **non-SPICE approximation**
4. Coupling high-pass 23 Hz (C5/R10)
5. Non-inverting op-amp with frequency-selective gain: 1 + 21.2766 * drive, using a 72 Hz high-pass in the feedback boost branch (C8/R13); additional approximate 18 kHz bandwidth limit
6. 7.2 kHz pre-clip low-pass approximation (R14/C10 **only in small-signal conditions**; real diode conduction changes effective cutoff)
7. D4/D5 shunt clipping, stock silicon or selected circuit-mod diode arrangement. Manual clipping overrides at D4/D5.
8. Passive low-pass 234 Hz (R16/C12), high-pass 1063 Hz (C11/R17) blended by Tone, with approximate -12 dB insertion loss.
9. Output coupling 3.4 Hz approximation.

## Circuit modifications
The existing Keeley Seeing Eye and Ultra options currently substitute the D4/D5 diode arrangements and retain a simplified bass adjustment from the common new-pedal input filter. **They do not yet include all published resistor/capacitor substitutions and are not complete replicas.** Stock DS-1A is the priority reference. The common model mod 3 and 4 toggles still add generic saturation and clean blend after the circuit; these are not hardware DS-1 mods and should eventually be removed or replaced with documented component patches.

## Calibration and test checklist
- Export reference SPICE frequency response with D4/D5 open and linear drive, plus full nonlinear transients with 100 mVpp and 200 mVpp input.
- Check Q2 headroom and asymmetric transfer at multiple amplitudes.
- Compare tone at 0, 25, 50, 75 and 100%, checking notch position near 500 Hz and nominal passive insertion loss.
- Compare Drive at 0, 25, 50, 75 and 100%, including low-frequency gain shelving and C10 loading under clipping.
- Measure THD and output RMS at 82.4 Hz, 440 Hz and 1 kHz, plus a multitone test.
- Validate each named mod against its exact published patch list and explicitly confirm automatic Follow Design clipping.
- Run Windows VST3 CI and perform DAW audition.

## Known limits
No SPICE netlist, numerical fitting, real-unit measurement, or audio A/B validation has been completed yet. One-pole approximations do not reproduce loading, tone pot resistance, diode dynamic resistance or bias shifts. The input and output shared filters, transformer and boost sections remain independent of this DS-1 model.
