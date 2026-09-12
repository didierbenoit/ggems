# I-125 selected-source validation reference

This reference was recovered from the retained raw scientific files before
inspection of the I-125 production arrays, tests or compiled export. It validates
the four selected prompt marginal-emission groups. Physical yields are emitted
particle multiplicities per parent decay, not mutually exclusive branch
probabilities; none is normalized to one.

## Authority, dates and decay scheme

The primary authority is V. Chiste, E. Schonfeld and M. M. Be, LNHB/DDEP
I-125/Te-125. [The evaluation comments](../raw/lnhb/I-125_com.pdf) state completion
in July 2010, including literature through July 2010. The
[PenNuc export](../raw/lnhb/I-125.PenNuc.txt) is dated 12/04/2010. The numerical
[tables](../raw/lnhb/I-125_tables.pdf) carry 23/01/1998 - 7/9/2010; their decay
drawing carries 23/01/1998 - 12/04/2010. The retained ENSDF-format LNHB input has
history cutoffs 30-AUG-2010, 24-JUL-2002 and 02-JAN-1998. These are distinct
documentary dates.

Selected half-life: **59.388(28) d**, or **5131123.2 +/- 2419.2 s** using exactly
86400 s/d. The quoted uncertainty was expanded by the evaluators to include the
most precise measurement; no new uncertainty fit is performed. LARA and the
retained BetaShape input separately print the rounded **5.1311E6(24) s**.
Selected Q(EC), also labeled Q+, is **185.77(6) keV**.

The adopted scheme is 100% allowed EC from I-125 ground state (5/2+) to the
**35.4922(5)-keV Te-125 level (3/2+)**, then the stable ground state (1/2+).
Comments section 2 gives its half-life as **1.48(1) ns**; PenNuc records 1.48E-9 s.
GGEMS flattens this de-excitation and its associated relaxation at parent decay
time. This is a source-model approximation, not a zero physical level lifetime.

Comments section 1 says direct ground-state EC was not observed (experimental
limit 0.01 per decay; systematics around 1E-6). Possible 144.8-keV feeding is
estimated below 1E-8 per decay. The evaluators consider the adopted scheme
complete. No extra branch is introduced. EC is not an incident particle; capture
neutrinos and recoil nuclei are excluded. No beta, positron, EC placeholder or
511-keV source photon is selected. Te-125 ground state is stable.

## BetaShape supporting EC calculation

The exact [retained command](../raw/betashape/v2.4/command.txt) is:

```text
.\betashape.exe I125\I-125.txt -csv
```

BetaShape **2.4 (06/2024)** supplies an allowed EC calculation at 150.28(6) keV,
with input intensity 100%. There is **no fixint=1**, no beta spectrum, no endpoint
mapping and no new calculation. Its calculated shell probabilities are
comparisons, not replacements for evaluated emission yields.

| Shell | Selected LNHB/PenNuc | BetaShape 2.4 calculation |
| --- | ---: | ---: |
| K | 0.8011(17) | 0.79926(41) |
| L | 0.1561(13), compact LNHB | 0.15563(15) |
| M | 0.0349(7) | 0.03571(16) |
| N | 0.0079(4), PenNuc CN | 0.00808(8) |
| O | No separate PenNuc capture record | 0.001319(22) |

PenNuc resolves CL1 = 0.1520(15) and CL2 = 0.0041(1), summing to 0.1561;
their uncertainties are not independently recombined. The retained
[capture report](../raw/betashape/v2.4/output/capt_I125_trans0.bs) also contains
subshell results. Calculated log ft is **5.4502(38)**, versus input 5.4.
LNHB selected captures were calculated using Schonfeld's EC-Capture tables;
the newer BetaShape calculation uses its own treatment.

## Selected emission inventory

| Group | Particle / law | Signatures | Yield per parent | Authority |
| --- | --- | ---: | ---: | --- |
| 0 | Gamma / Mono | 1 | 0.0663 | LNHB gamma evaluation; LARA and PenNuc GA |
| 1 | Gamma / DiscreteLines | 5 | 1.5264 | Compact LARA X rays |
| 2 | Electron / DiscreteLines | 13 | 23.008143718 | MIRDspecs / ICRP 107 Auger rows |
| 3 | Electron / DiscreteLines | 6 | 0.93378 | LNHB/PenNuc conversion records |
| Total | | 25 | **25.534623718** | Unnormalized sum |

The nuclear gamma is 35.4922(5) keV with yield 0.0663(6).
[LARA](../raw/lnhb/I-125.lara.txt) contains exactly six photon signatures: this
gamma plus the five compact X-ray entries below. It does not contain the
conversion-electron table.

| Compact X-ray signature | Selected energy (keV) | Yield per parent |
| --- | ---: | ---: |
| XL | 4.0788 | 0.1470(28) |
| K-alpha-2 | 27.202 | 0.393(5) |
| K-alpha-1 | 27.4726 | 0.732(8) |
| K-beta-1 group | 31.0589 | 0.209(3) |
| K-beta-2 group | 31.7623 | 0.0454(13) |

XL summarizes 3.3348-4.8228 keV. The first K-beta group contains 30.9446,
30.996 and 31.236 keV components; the second contains 31.7008, 31.774 and
31.812 keV. Representative energies and summed intensities are taken directly
from LARA, not reconstructed as invented microscopic lines.

## Detailed Auger source selection

All 13 rows labeled `Auger electron` in the retained
[I-125 Summary Spectrum.csv](../raw/mird/I-125%20Summary%20Spectrum.csv) are selected:
zero-based source rows 50-58 and 60-63. Row 59 is a conversion electron and is
not part of this selection. The [official MIRDspecs source page](https://mirdsoft.org/products/MIRDspecs/MIRDspecs_HTMLs/I-125.htm)
identifies ICRP Publication 107 as its spectra source.

ICRP 107 describes outer-shell Auger/Coster-Kronig cascade calculations and
summary channels, with yields in particles per nuclear transformation and
unique or average energies. Thus these 13 summary signatures are more detailed
than the LNHB compact K/L inventory; they are not a complete microscopic line
list. See [ICRP 107, sections 2.6 and 3.2.2-3.2.4](https://journals.sagepub.com/doi/pdf/10.1177/ANIB_38_3).

The retained selected yields sum to **21.214952128 below 1 keV**, **1.5984545
in the L range**, and **0.19473709 in the K ranges**. The outer-shell cascade
therefore explains the large total. LNHB instead gives L Auger **1.582(8)** at
2.3-4.8 keV and K Auger **0.197(7)** across KLL 21.804-22.989, KLX
25.814-27.470 and KXY 29.80-31.81 keV. Relative K-family probabilities are
100 : 45.3 : 5.13. The models are not identical, even in their K/L subsets;
their totals are neither equated nor added together. LNHB atomic data cite
Schonfeld and Janssen, NIM A369 (1996) 527; its X-ray intensities use EMISSION.

MIRD Auger electrons and LNHB X-ray photons are distinct emitted marginals.
MIRD gamma, X-ray and conversion rows are not imported. The selected conversion
records are separate from the Auger rows; no particle signature is duplicated.
This is a deliberate mixed-source marginal model, not a single identical atomic
calculation or an event-correlated vacancy cascade.

## Conversion scope and the EN record

| PenNuc record | Energy (keV) | Yield per parent |
| --- | ---: | ---: |
| EK | 3.6784 | 0.776(13) |
| EL1 | 30.553 | 0.0936(16) |
| EL2 | 30.8802 | 0.0172(20) |
| EL3 | 31.1508 | 0.0159(33) |
| EM | 34.7224 | 0.0256(11) |
| EN | 35.3985 | 0.00548(22) |

Each PenNuc electron energy carries 0.0005-keV uncertainty. These explicit
records determine the selected K/L1/L2/L3/M/N representation. The compact LNHB
table gives L = 0.127(5), consistent with the PenNuc L sum 0.1267 after rounding.

The compact **N = 0.00497(20)** differs from **PenNuc EN = 0.00548(22)**.
This is not merely last-digit rounding. LNHB comments table 4 lists
alpha_N = 0.075(3) and alpha_O = 0.00766(23):

```text
0.0663 * alpha_N             = 0.0049725
0.0663 * (alpha_N + alpha_O) = 0.005480358
```

The latter rounds to the explicit EN value. This supports an **inference of
outer-shell aggregation**; the retained files do not explicitly describe its
algorithm or establish a later update. The reference preserves the documented
EN energy and yield without relabeling it as a separately resolved O line.
Neither the compact electron inventory nor PenNuc supplies an EO emitted-electron
entry. No additional particle is inferred from the O coefficient. Adding one
could count the same outer-shell contribution again. The source discrepancy
remains documented rather than averaged or hidden.

## Representation and independent cross-checks

`reference.json` retains every selected physical line yield and source row.
For MIRD energies it also preserves the original MeV and keV values. The existing
historical nearest-meV representation changes 0.0229271 to 0.022927 keV and
0.0249224 to 0.024922 keV: residuals **-100 and -400 micro-eV**. All other selected
energies are exact in integer micro-eV. `energy_keV` documents that representation
for discrete-law comparison; the raw values remain available for the energy
audit. Conditional ticket normalization never changes a physical group yield.

The retained [ENSDF evaluation](../raw/ensdf/ec_decay.pdf), J. Katakura,
NDS112 (2011) 495, cutoff January 1, 2010, gives 59.400(10) d, 35.4925(5) keV,
gamma yield 0.0668(13), and a different mixing ratio/ICC set. Those values do
not replace selected LNHB data. MIRD likewise uses its own gamma yield 0.0668,
X rays and conversion model (for example K conversion 3.6872 keV, yield
0.807993). The legacy MIRD-format table and DPK file are not the selected Auger
source. No alternative emission dataset is silently combined with this one.
