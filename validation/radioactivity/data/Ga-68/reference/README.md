# Selected Ga-68 reference

This package maps the recovered LNHB/PTB evaluation and BetaShape 2.4 output to
the seven existing GGEMS source groups. It supplies data to the existing generic
radioactivity validator. Raw paths below are relative to `../raw/`.

## Nuclear selection and provenance

The authority is the evaluation by M.-M. Be (LNHB) and E. Schonfeld (PTB), reviewed
in November 2011 according to `lnhb/Ga-68_com.pdf`. The recovered tables have the
footer `21/01/1998 - 4/7/2012`; the PenNuc file prints `09/12/2011`. These source
dates are retained without inventing download or execution dates.

- Half-life: **67.83(20) min = 4069.8 s**, with a reported 12.0 s uncertainty.
  The evaluation comments call 0.20 min an expanded uncertainty chosen to cover
  inconsistent measurements. No coverage factor is given; it is not relabeled
  as a standard uncertainty. LARA's rounded `4.070E3 s` is not substituted.
- Q+: **2921.1(12) keV**.
- Evaluated totals: **88.88(41)% beta+**, **11.11(41)% EC**. The explicit three
  beta+ yields sum to 0.8888026 per parent decay. The rounded totals and branch
  values are preserved separately; no normalization repairs their rounding.
- Annihilation: **177.8(8) photons per 100 disintegrations** at 511 keV is
  reference information only. GGEMS source emissions must exclude that line.

`reference.json` preserves decimal values, quoted uncertainties, source files,
line numbers and transformations. Discrete-group yields are exact decimal sums
of their selected line yields. No uncertainty on a sum is inferred without
covariance. A missing line uncertainty remains null or absent.

## Ordered source mapping

| Index | Particle | Distribution | Selected content | Yield per parent decay |
|---:|---|---|---|---:|
| 0 | Positron | RegularSpectrum | 1899.1 keV endpoint, experimental column | 0.8768 |
| 1 | Positron | RegularSpectrum | 821.75 keV endpoint, calculated column | 0.0120 |
| 2 | Positron | RegularSpectrum | 243.23 keV endpoint, calculated column | 0.0000026 |
| 3 | Gamma | DiscreteLines | 13 direct nuclear gamma lines | 0.03611589 |
| 4 | Gamma | DiscreteLines | 4 compact Zn X-ray lines | 0.04919 |
| 5 | Electron | DiscreteLines | 9 detailed Auger lines | 0.41082910426 |
| 6 | Electron | DiscreteLines | 78 shell-resolved conversion lines | 0.000009159860474 |

The total is **1.384946754120474 primaries per parent decay**. These groups are
emission multiplicities, not mutually exclusive parent branches. EC creates no
placeholder incident particle or extra multiplier on relaxation yields.

Use LARA's direct photon emission energies rather than nuclear level differences.
The compact XL and K-beta entries retain LARA's representative energies without
inventing sub-lines. The possible 1655.87 keV E0 transition has no selected direct
photon intensity and is not fabricated as a gamma source.

Detailed Auger lines alone use `mird/Ga-68 Summary Spectrum.csv`, rows 42-50. This
is the explicit existing data boundary documented by `GGEMSGa68.cc`: LNHB gives
Auger groups as energy ranges, while the built-in takes detailed MIRD lines.
The production comment identifies MIRDspecs/ICRP Publication 107 (2008); the CSV
itself supplies no evaluation identity or uncertainties. All other MIRD data and
ENSDF remain independent cross-checks; their evaluations are not merged here.

The Auger mapping retains the documented historical nearest-meV quantization:
`0.0567753 -> 0.056775 keV` and `0.0898614 -> 0.089861 keV`, differences of -300
and -400 micro-eV. All other selected discrete energies are unchanged. The JSON
retains original MeV/keV values beside the mapped `energy_keV` read by the
validator. Agreement with that mapping does not mean exact agreement with the
two unquantized MIRD line locations. Canonical GGEMS Energy remains uint64
micro-eV, with exactly 1,000,000,000 micro-eV per keV. Dose is unchanged.

## BetaShape selection and transformation

The recovered `betashape/v2.4/command.txt` records:

```powershell
.\betashape.exe Ga68\Ga-68.txt -fixint=1 -csv
```

The input is `betashape/v2.4/input/Ga-68.txt`. Output headers identify
**BetaShape 2.4 (06/2024)**, X. Mougeot, and Applied Radiation and Isotopes 201,
111018 (2023). `betashape/v2.4/output/Ga-68.rpt` confirms that `fixint=1` retains
the adopted EC/beta+ intensities. This package reads those outputs; it does not
rerun BetaShape.

| Group | Recovered output | Density column | Points | Raw endpoint -> selected endpoint (keV) |
|---:|---|---|---:|---|
| 0 | `beta+_Ga68_trans0.bs` | `dN/dE exp.` and its uncertainty | 318 | 1899.1 -> 1899.1 |
| 1 | `beta+_Ga68_trans1.bs` | `dN/dE calc.` and its uncertainty | 412 | 821.8 -> 821.75 |
| 2 | `beta+_Ga68_trans2.bs` | `dN/dE calc.` and its uncertainty | 305 | 243.2 -> 243.23 |

All three outputs are under `betashape/v2.4/output/`. The dominant experimental
shape factor is `1 - 0.01*W`; its header cites W.F. Slot et al., Nucl. Phys. A 186,
28 (1972), and a measured range of 870-1845 keV. The selected reference uses the
entire provided experimental table, not the calculated column or a clipped range.

Only the energy axes of the two calculated spectra are scaled to the selected
PenNuc endpoints. The CSV energy decimals use 80-digit arithmetic; supplied
density and uncertainty tokens are retained without alteration. Interpolation is
piecewise linear. Its full integral is normalized only for the conditional
energy law; the branch yield remains the independent physical value above.
Integrals and conditional means are recorded in the JSON.

Supplied pointwise uncertainties are preserved. No covariance is available, so
the validator compares central shapes without claiming uncertainty-aware
equivalence of evaluations. GGEMS regular grids and finite tickets introduce
small representation residuals that the deterministic comparison reports.

The current built-in comment identifies a retained BetaShape 2.2 experimental
table for group 0 and updated BetaShape 2.4 calculated tables for groups 1-2.
The exporter has no compiled provenance API. Source-comment provenance and
numerical comparison are separate; a retained generator label alone does not
establish a spectral discrepancy.

## Existing campaign

From the source root, after building `validation_radioactivity` in an isolated
source copy where required by workspace protection:

```powershell
python ./validation/radioactivity/run_campaign.py --exporter ./build/validation/radioactivity/ggems_radionuclide_exporter.exe --reference ./validation/radioactivity/data/Ga-68/reference/reference.json --output ./codex_scratch/ga68_cpu --device cpu
python ./validation/radioactivity/plot.py ./codex_scratch/ga68_cpu
```

Use a new output directory and `--device gpu` for a GPU campaign. Existing defaults
are four half-lives, 32 windows, 1000 expected primaries in the final window,
Philox seed 77777, 256 workers, 128 separate host population replicas, and family
alpha 0.01. Activity is selected automatically from the original total yield.
Rare groups are not amplified. Report insufficient energy statistics as such.
Read `analysis.json` separately for nuclear mapping, integrated decay counts,
Poisson populations, conditioned birth times and energy distributions.
Physical daughter transport, annihilation, stopping and dose are outside scope.
