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
