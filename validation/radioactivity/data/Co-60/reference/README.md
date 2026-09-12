# Co-60 selected source reference

This package reconstructs the selected Co-60 independent-particle emission
definition from the recovered files in `../raw/`. Raw scientific values and the
three spectrum CSVs were recovered before inspecting the current built-in.
The JSON uses the existing generic reference format.

## Selected authority

The primary nuclear evaluation is R. G. Helmer's Co-60 / Ni-60 evaluation,
reviewed/updated by M.-M. Be through January 2006. See
`lnhb/Co-60_com.pdf`, opening paragraph; `lnhb/Co-60.PenNuc.txt`, evaluation
date 13/01/2006; and `lnhb/Co-60.txt`, update card 16-JAN-2006.
The retained tables PDF also carries later table-format footer dates.

- **Half-life:** LARA gives **5.2711(8) a** and explicitly
  **166.340E6(0.025E6) s**. This package selects the explicit seconds value,
  **166340000 +/- 25000 s**, retaining the evaluated years separately.
  It does not substitute a newly calculated year-to-second conversion.
  The compiled half-life is exactly 166340000 s and matches the existing
  relative scalar tolerance of 1e-14. Half-life is a host scalar, distinct
  from the bounded integer-picosecond Run chronology.
- **Q value:** 2823.07(21) keV, from the selected LNHB evaluation and PenNuc.
- **Beta endpoints/yields:** the three PenNuc `BEM` records and LNHB tables
  section 2.1. All endpoint standard uncertainties are 0.21 keV.
- **Nuclear photons and compact Ni X-rays:** the ten LARA photon rows.
- **Conversion electrons:** all twelve PenNuc `EK`/`EL` records, sorted by
  electron energy. These are emitted-electron yields, not coefficients to
  multiply by the gamma yield a second time.
- **Detailed Auger electrons only:** all seven `Auger electron` rows in
  `mird/Co-60 Summary Spectrum.csv`. LNHB supplies grouped atomic ranges,
  not these seven lines. The established MIRDspecs provenance identifies
  ICRP Publication 107; the retained CSV contains numerical rows but does
  not itself embed that bibliographic attribution.

ENSDF and MIRD nuclear quantities do not override LNHB/LARA/PenNuc. MIRD
contributes neither beta branches, photons nor conversion electrons here.

## BetaShape 2.4 selection

The retained command in `betashape/v2.4/command.txt` is:

```powershell
.\betashape.exe Co60\Co-60.txt -csv
```

Only the retained **2.4 (06/2024)** outputs are active evidence. No `-qval`,
`fixint`, endpoint rescaling or BetaShape rerun is used.

| Group | Endpoint keV | Physical yield/parent | Retained transition | Selected column | Model/provenance |
|---:|---:|---:|---|---|---|
| 0 | 317.32 | 0.9988 +/- 0.0003 | `beta-_Co60_trans2.bs` | `dN/dE exp.` | Allowed (`A`); `Cexp(W)=1`, `1972SA**` |
| 1 | 664.46 | 0.00002 | `beta-_Co60_trans1.bs` | `dN/dE calc.` | Unique second forbidden (`2U`); no tabulated experimental factor |
| 2 | 1490.56 | 0.0012 +/- 0.0003 | `beta-_Co60_trans0.bs` | `dN/dE exp.` | `2U`; experimental factor, `1956WO09` |

For group 0, BetaShape cites K. S. R. Sastry, Massachusetts University Report
AD-752621 (1972), measurement range 75-280 keV, database transition 34.
For group 2 it cites J. L. Wolfson, Canadian Journal of Physics 34, 256 (1956),
measurement range 1350-1450 keV, database transition 35. Its selected factor is
`q^4 + (10/3)*lambda_2*q^2*p^2 + lambda_3*p^4`.
The retained report's experimental means are 95.52(7) and 624.50(9) keV;
the calculated alternatives are 94.73(7) and 623.03(9) keV and are not selected.
The weak calculated branch reports 273.60(9) keV.
The retained `.trans` records warn that information about these decays is
not totally sure. No transition-model correction is introduced.

Each CSV copies the retained energy, selected density and uncertainty tokens
unchanged: respectively **319, 334 and 374 knots**, including zero and the
selected endpoint. Piecewise-linear integration over the full support gives
conditional means **95.524243851614, 273.599460849367 and
624.504291110055 keV**. Conditional normalization divides by the integrated
density, independently of the physical group yield. Pointwise uncertainties
remain available; no covariance information is supplied for an
uncertainty-aware shape-equivalence test.

Current production comments record the two experimental tables as historically
retained from BetaShape 2.2. The active 2.4 experimental columns reproduce
their conditional bin weights to about 1e-16 and every finite-ticket count.
This establishes numerical continuity without using 2.2 files as active data.
The 664.46 keV table reproduces the selected 2.4 calculated column.

| Group | Compiled half-open support keV | Bins | Width keV | Finite-ticket mean keV | Full grid/reference CDF supremum |
|---:|---|---:|---:|---:|---:|
| 0 | [0.000340, 317.32) | 635 | 0.499716 | 95.524596004195 | 2.82096543e-6 |
| 1 | [0.002528, 664.46) | 1329 | 0.499968 | 273.600672771235 | 5.48975077e-6 |
| 2 | [0.001336, 1490.56) | 2982 | 0.499852 | 624.504779194495 | 7.64997900e-7 |

The reference keeps the full [0, endpoint] support. The historical regular
grids exclude tiny lower-tail masses: 2.24710359e-6, 4.29990083e-6 and
7.64997900e-7. For the separate representation audit, integrating the retained
density over each compiled bin and conditioning on that grid reproduces all
**4946 ticket counts exactly**. The generic full-reference comparison does
not remove those tails. The finite integer-energy CDF differs from its
continuous grid envelope by at most one 2^-32 ticket quantum.

## Discrete groups and representation

| Group | Particle / content | Lines | Physical group yield per parent |
|---:|---|---:|---:|
| 3 | Gamma / direct nuclear photons | 6 | 1.99848902 |
| 4 | Gamma / compact Ni X-rays | 4 | 0.000114 |
| 5 | Electron / detailed Auger | 7 | 0.00121289017 |
| 6 | Electron / K/L conversion | 12 | 0.000292517293712 |

All **29** energies, line yields and group memberships are retained in JSON.
All 29 independently apportioned DiscreteLines ticket counts match the compiled
export and are positive, including the weakest conversion lines.
The dominant 1173.228 and 1332.492 keV photon yields remain 0.9985 and
0.999826. The gamma multiplicity near two reflects the physical cascade;
the flattened source does not reconstruct coincident cascade events.

The compact 0.84 keV XL and 8.2967 keV K-beta energies are the effective LARA
entries. Microscopic sub-lines are not invented. Two MIRD Auger energies retain
historical integer-meV rounding, now stored as integer micro-eV:

| Raw keV | Represented keV | Residual micro-eV |
|---:|---:|---:|
| 0.0479803 | 0.047980 | -300 |
| 0.0787867 | 0.078787 | +300 |

JSON preserves `source_energy_MeV`, `source_energy_keV` and the residual;
`energy_keV` is the declared sampling representation. Other selected energies
are exactly representable. Line `weight` values retain absolute physical
yields; the energy sampler normalizes them only conditionally within a group.
The beta yield sum is **1.00002** and the total flattened yield is
**3.000128427463712**. Neither is normalized to one.

## Internal-pair boundary

LNHB comment sections 2.2 and 4.2 give internal-pair coefficients
`alpha_p(1173)=0.0000062(7)` and `alpha_p(1332)=0.000034(4)` and account for
them when deriving the selected direct photon intensities. The tables PDF
section 4 gives approximately **310.51(1) keV total pair kinetic energy** for
the 1332-keV channel. It is not one electron's or positron's energy.

These selected channels are outside the current independent-particle source
representation, which has no correlated electron/positron energy-sharing law.
No fake mono electron, positron, gamma, or duplicated full-energy pair is
introduced. All seven compiled groups map to the independent selected data;
there are no positron or Mono groups and no 310.51 keV discrete line.
This intentional boundary is not missing nuclear data. Direct gamma yields
are not increased to compensate for excluded pairs.

## Source disagreements and documentary details

- The LNHB comment section 2.1 discusses the weak beta branch as
  **0.000(2)%**; the selected tabulated evaluation, PenNuc and retained
  BetaShape input use **0.002%**. This package preserves the explicitly
  selected positive central value without converting that comment's
  uncertainty into a new adopted policy.
- The recovered ENSDF evaluation (December 2012, Browne/Tuli) gives
  **1925.28(14) d**, Q **2822.8(2) keV**, beta entries **317.88(10),
  670(20), 1492(20) keV**, and the weak intensity **0.000(2)%**. It calls
  that branch's existence questionable. These do not replace LNHB values.
- The LNHB PDF electron section lists K-conversion energies 1164.895 and
  1324.157 keV. Selected PenNuc gives 1164.9072 and 1324.1752 keV, differences
  +0.0122 and +0.0182 keV. The reference explicitly selects all twelve PenNuc
  K/L rows; no average or silent reconciliation is made.
- Production comments attribute the 310.51 keV entry and K/L rows broadly to
  LARA. In the recovered evidence, the pair entry is in the **tables PDF**
  and the twelve electron rows are in **PenNuc**; the `.lara.txt` contains
  photons only. Production comments are unchanged.

For campaign use, the existing Time-range policy caps the requested four
half-lives at about 90% of uint64 picoseconds. Rare groups retain their
physical yields; empty or non-discriminating DKW comparisons remain
`insufficient_samples`. No special sampling campaign is implied by this package.
