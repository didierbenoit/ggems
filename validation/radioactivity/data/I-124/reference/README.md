# I-124 independent reference

The selected authority is the B. E. Zimmerman LNE-LNHB / NIST DDEP
I-124 / Te-124 evaluation updated in 2021. All scientific values and spectra
were recovered from [the local raw evidence](../raw/) before inspecting the
I-124 production definition. Production was subsequently used to audit the
existing numerical representation and group mapping, not as nuclear-data input.

## Authority and parent definition

- `lnhb/I-124_tables.pdf`: evaluation tables, footer dated
  `16/03/2016 - 20/7/2021`; sections 2, 2.1-2.2, 5.1-5.2 and 6.
- `lnhb/I-124_com.pdf`: Zimmerman evaluation comments, completed in 2016 and
  updated to include 2021PI01 and 2021WA16. Quoted uncertainties are combined
  uncertainties at one standard deviation.
- `lnhb/I-124.lara.txt`: NIST 2021 compact evaluated photon emissions.
- `lnhb/I-124.txt` and `lnhb/I-124.PenNuc.txt`: the associated evaluated decay
  records, including the three exceptionally weak beta-plus branches omitted
  from the printed section 2.2 table. PenNuc is dated 15/07/2021.

The selected half-life is **4.17600(29) d = 360806.40000 s**, with uncertainty
**25.05600 s**. Days and their uncertainty are multiplied by exactly 86400;
the rounded LARA seconds field is not substituted. The selected Q value is
**3159.6(19) keV**, from the evaluation and PenNuc. Q is spectrum provenance;
the compiled radionuclide definition has no separate Q-value field.

## Beta-plus spectra and EC splitting

The retained command in `betashape/v2.4/command.txt` is exactly:

```text
.\betashape.exe I124\I-124.txt fixint=1 -csv
```

The retained input matches `lnhb/I-124.txt`. All 28 evaluated EC/beta-plus
intensity pairs in its E cards are preserved in `output/I-124.new`.
`fixint=1` preserves those adopted splits while BetaShape recalculates spectral
and capture information. Recalculated capture-shell probabilities do not
replace the evaluated physical beta-plus yields. EC itself creates no
placeholder incident particle.

All quantities below are per parent decay. Endpoints have selected uncertainty
1.9 keV. The retained BetaShape endpoints use the evaluation's 0.1 keV display
precision; the more precise PenNuc endpoint tokens are also retained in JSON.

| Group / transition file | Endpoint keV | Beta-plus yield | Yield uncertainty | Associated EC yield | Selected shape |
|---|---:|---:|---:|---:|---|
| 0 / trans0 | 2137.6 | 0.1032 | 0.0013 | 0.2408 | Experimental |
| 1 / trans1 | 1534.9 | 0.1145 | 0.0015 | 0.2555 | Experimental |
| 2 / trans2 | 889.0 | 1.88e-6 | 1.6e-7 | 0.000118 | Calculated |
| 3 / trans3 | 812.1 | 0.00287 | 0.0001 | 0.0543 | Calculated |
| 4 / trans4 | 480.3 | 1.21e-6 | 1.0e-7 | 0.00106 | Calculated |
| 5 / trans5 | 254.7 | 1.5e-8 | 9e-9 | 0.00025 | Calculated |
| 6 / trans6 | 98.3 | 1.8e-9 | 1.1e-9 | 0.00026 | Calculated |
| 7 / trans7 | 46.0 | 2.5e-10 | 7e-11 | 0.00192 | Calculated |

The eight CSVs copy the energy, selected density and uncertainty tokens from
`betashape/v2.4/output/beta+_I124_trans0.bs` through `trans7.bs`, unchanged.
No BetaShape run, older version, `-qval`, endpoint rescaling, or physical-yield
normalization is used. The reference law is the full piecewise-linear density.
Its integral is divided out only for a conditional energy CDF or mean;
physical yields remain the independently selected values above.

The experimental columns for groups 0 and 1 use Booij et al., Nucl. Phys. A
160, 337 (1971), `1971BO01`:

- 2137.6 keV: `(1 - 0.004*W) * (q^2 + l_2*p^2)`, measured over 1530-2000 keV,
  database transition 76; header means 968.7 keV calculated and 975.2 keV
  experimental.
- 1534.9 keV: `1 - 0.046*W`, measured over 820-1350 keV, database transition 75;
  header means 684.6 keV calculated and 681.9 keV experimental.

The full retained experimental columns are selected; their complete support is
not claimed to have been measured. Groups 2, 4 and 5 use first-forbidden unique
calculations. Groups 3, 6 and 7 use non-unique forbidden transitions calculated
as allowed under the Xi approximation. BetaShape warns that the 812.1 keV
shape is unpredictable and should be checked with measurement when possible;
it reports "should be correct" for 98.3 and 46.0 keV. Its analogous warning on
the calculated 1534.9 keV shape does not replace the selected experimental
column. These are retained model limitations, not corrections made by GGEMS.

## Discrete emissions and numerical mapping

| Group | Particle / content | Lines | Physical group yield | Authority |
|---|---|---:|---:|---|
| 8 | Gamma / prompt nuclear gamma | 85 | 0.984815 | LNHB/LARA emitted photon table |
| 9 | Gamma / compact Te X-rays | 5 | 0.6411 | LNHB/LARA |
| 10 | Electron / detailed Auger | 13 | 9.165447885 | MIRD Summary Spectrum CSV |
| 11 | Electron / conversion, main | 401 | 0.003555243101 | LNHB PenNuc |
| 12 | Electron / conversion, weak | 109 | 4.41368e-8 | LNHB PenNuc |

Gamma energies and yields use the emitted-photon values, not transition
energies or gamma-plus-conversion probabilities from section 2.3. Compact
X-ray energies use the five LARA representatives. Conversion data comprise
every positive `EK`, `EL1`, `EL2`, `EL3`, `EM`, and `EN` record in PenNuc.
Each line retains its source location, energy, physical yield, and available
uncertainties; conversion lines also retain shell and level assignments.

MIRD is used only for the 13 rows labeled `Auger electron` in
`mird/I-124 Summary Spectrum.csv`, because LNHB supplies grouped Auger
information rather than this detailed representation. No MIRD nuclear,
X-ray, beta, or conversion value overrides LNHB. The CSV provides no evaluated
uncertainties or release identifier; these are not invented. The production
comment attributes MIRDspecs to ICRP Publication 107 (2008), but the retained
CSV alone does not independently establish that attribution.

The union of groups 11 and 12 retains the full **510-line physical dataset**.
The existing numerical boundary is physical line yield `1e-9`: main is
`>= 1e-9`, weak is `< 1e-9`. Their exact sum is **0.0035552872378**; no line
or physical intensity is removed, redistributed, or normalized. Each group
has its own conditional 2^32-ticket representation. The split is not a new
physical decay channel.

One documentary qualification: the source describes the split as necessary
for ticket reachability. For the present selected dataset, however, the
minimum unsplit quota is `2^32 * 1.56e-11 / 0.0035552872378 = 18.84559118`.
All positive lines could receive tickets even without that split. The existing
split is preserved and audited; its reachability-only necessity is not asserted
as a scientific fact.

Canonical runtime energy is integer micro-eV. The built-in preserves its
historical integer-meV embedding. All selected discrete energies reproduce it
exactly except two documented Auger rounding residuals:

| Raw MIRD energy keV | Represented energy keV | Residual micro-eV |
|---:|---:|---:|
| 0.0229241 | 0.022924 | -100 |
| 0.0249253 | 0.024925 | -300 |

Original MIRD MeV tokens and changed raw keV values remain in the JSON.
`energy_keV` is the explicitly documented represented line location used by
the existing validator. Physical line weights are unchanged. Finite-ticket
line probabilities are separately audited against these physical weights.

For beta groups 6 and 7, production historically zeroed conditional bin masses
below 2^-32 offline: bins 0-1 and 0-5 respectively, with removed conditional
mass **9.8989605885e-12** and **4.5505072455e-11**. The retained reference CSVs
keep the full raw densities. Deterministic comparison measures the complete
reference discrepancy and separately reproduces the documented production
representation; no reference density or statistical threshold is adjusted.
The retained 46.0 keV table already has zero central density at 45.8, 45.9,
and 46.0 keV. Its final two compiled bins consequently carry zero tickets.
The nominal endpoint and the exactly sampleable ticket support are distinct;
this is reproduced from the retained output, not a new endpoint correction.

## Exclusions, source disagreements, and yield meaning

The LARA `g511` entry is annihilation radiation: 511 keV, yield 0.4411 per
decay. It is **excluded from the parent source and aggregate gamma emission
spectrum**. GGEMS emits the positrons themselves, so source-level annihilation
photons would double count subsequent annihilation. Recoil nuclei and
internal-pair formation are outside this selected source definition.

ENSDF is a cross-check only: Katakura / Wu, April 2008, NDS 109, 1655. It gives
4.1760(3) d and, for example, dominant beta-plus intensities 10.7(3)% and
11.7(2)% versus selected 10.32(13)% and 11.45(15)%. Its 889 keV intensity is
0.00065(16)% versus selected 0.000188(16)%. Values are not averaged or replaced.

The beta-plus yields sum to **0.22057310705**. The 28 evaluated E-card EC
intensities sum to **0.777288**, while the separately rounded PenNuc capture
shell records sum to **0.7773684326**; these representations are not silently
reconciled. The selected flattened particle yield is **11.0154912792878**.
Emission multiplicities are not categorical branch probabilities. Neither
physical group yields nor physical line yields are forced to sum to one.

Very weak branches and lines remain in the reference even with zero observed
primaries. The existing empty-sample / DKW-limit qualification applies without
amplification or oversampling. Deterministic validation remains available when
statistical energy validation is `insufficient_samples`.
