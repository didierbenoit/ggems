#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSMaterial.hh"
#include "GGEMS/materials/builtins/GGEMSBuiltInMaterials.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace ggems::core::materials::builtins {

namespace {

// =============================================================================
// =============================================================================

using namespace ggems::units;

using Component = GGEMSMaterialComponent;

// =============================================================================
// =============================================================================

struct ElementalMaterialRow {
  std::string_view canonical_name;
  std::uint32_t atomic_number;
  Density density;
};

// =============================================================================
// =============================================================================

struct CompoundMaterialRow {
  std::string_view canonical_name;
  Density density;
  std::span<Component const> composition;
};

// =============================================================================
// =============================================================================

// Elemental-material densities:
// NIST X-Ray Mass Attenuation Coefficients, Table 1.
// https://physics.nist.gov/PhysRefData/XrayMassCoef/tab1.html
//
// Air, Water, CdTe, CsI, GaAs, and GOS:
// NIST X-Ray Mass Attenuation Coefficients, Table 2.
// https://physics.nist.gov/PhysRefData/XrayMassCoef/tab2.html
//
// Medical tissues, LSO, and NaI preserve the useful GGEMS 1.3 material
// definitions. The tissue set is intentionally application-oriented rather
// than a wholesale NIST material import. In particular, GGEMS "Lung" is the
// legacy bulk lung definition at 0.26 g/cm3.
//
// Canonical Material names are intentionally short public API tokens suitable
// for SetMaterial(): Brain, Breast, LSO, CdTe, etc. Source labels and
// provenance belong in comments, not in runtime names.

// =============================================================================
// =============================================================================

constexpr std::array<ElementalMaterialRow, 92U> k_elemental_materials{{
    {.canonical_name = "Hydrogen",
     .atomic_number = 1U,
     .density = 8.375e-5_g_cm3},
    {.canonical_name = "Helium",
     .atomic_number = 2U,
     .density = 1.663e-4_g_cm3},
    {.canonical_name = "Lithium",
     .atomic_number = 3U,
     .density = 5.340e-1_g_cm3},
    {.canonical_name = "Beryllium",
     .atomic_number = 4U,
     .density = 1.848e+0_g_cm3},
    {.canonical_name = "Boron", .atomic_number = 5U, .density = 2.370e+0_g_cm3},
    {.canonical_name = "Carbon",
     .atomic_number = 6U,
     .density = 1.700e+0_g_cm3}, // NIST: Carbon, Graphite.
    {.canonical_name = "Nitrogen",
     .atomic_number = 7U,
     .density = 1.165e-3_g_cm3},
    {.canonical_name = "Oxygen",
     .atomic_number = 8U,
     .density = 1.332e-3_g_cm3},
    {.canonical_name = "Fluorine",
     .atomic_number = 9U,
     .density = 1.580e-3_g_cm3},
    {.canonical_name = "Neon", .atomic_number = 10U, .density = 8.385e-4_g_cm3},
    {.canonical_name = "Sodium",
     .atomic_number = 11U,
     .density = 9.710e-1_g_cm3},
    {.canonical_name = "Magnesium",
     .atomic_number = 12U,
     .density = 1.740e+0_g_cm3},
    {.canonical_name = "Aluminum",
     .atomic_number = 13U,
     .density = 2.699e+0_g_cm3},
    {.canonical_name = "Silicon",
     .atomic_number = 14U,
     .density = 2.330e+0_g_cm3},
    {.canonical_name = "Phosphorus",
     .atomic_number = 15U,
     .density = 2.200e+0_g_cm3},
    {.canonical_name = "Sulfur",
     .atomic_number = 16U,
     .density = 2.000e+0_g_cm3},
    {.canonical_name = "Chlorine",
     .atomic_number = 17U,
     .density = 2.995e-3_g_cm3},
    {.canonical_name = "Argon",
     .atomic_number = 18U,
     .density = 1.662e-3_g_cm3},
    {.canonical_name = "Potassium",
     .atomic_number = 19U,
     .density = 8.620e-1_g_cm3},
    {.canonical_name = "Calcium",
     .atomic_number = 20U,
     .density = 1.550e+0_g_cm3},
    {.canonical_name = "Scandium",
     .atomic_number = 21U,
     .density = 2.989e+0_g_cm3},
    {.canonical_name = "Titanium",
     .atomic_number = 22U,
     .density = 4.540e+0_g_cm3},
    {.canonical_name = "Vanadium",
     .atomic_number = 23U,
     .density = 6.110e+0_g_cm3},
    {.canonical_name = "Chromium",
     .atomic_number = 24U,
     .density = 7.180e+0_g_cm3},
    {.canonical_name = "Manganese",
     .atomic_number = 25U,
     .density = 7.440e+0_g_cm3},
    {.canonical_name = "Iron", .atomic_number = 26U, .density = 7.874e+0_g_cm3},
    {.canonical_name = "Cobalt",
     .atomic_number = 27U,
     .density = 8.900e+0_g_cm3},
    {.canonical_name = "Nickel",
     .atomic_number = 28U,
     .density = 8.902e+0_g_cm3},
    {.canonical_name = "Copper",
     .atomic_number = 29U,
     .density = 8.960e+0_g_cm3},
    {.canonical_name = "Zinc", .atomic_number = 30U, .density = 7.133e+0_g_cm3},
    {.canonical_name = "Gallium",
     .atomic_number = 31U,
     .density = 5.904e+0_g_cm3},
    {.canonical_name = "Germanium",
     .atomic_number = 32U,
     .density = 5.323e+0_g_cm3},
    {.canonical_name = "Arsenic",
     .atomic_number = 33U,
     .density = 5.730e+0_g_cm3},
    {.canonical_name = "Selenium",
     .atomic_number = 34U,
     .density = 4.500e+0_g_cm3},
    {.canonical_name = "Bromine",
     .atomic_number = 35U,
     .density = 7.072e-3_g_cm3},
    {.canonical_name = "Krypton",
     .atomic_number = 36U,
     .density = 3.478e-3_g_cm3},
    {.canonical_name = "Rubidium",
     .atomic_number = 37U,
     .density = 1.532e+0_g_cm3},
    {.canonical_name = "Strontium",
     .atomic_number = 38U,
     .density = 2.540e+0_g_cm3},
    {.canonical_name = "Yttrium",
     .atomic_number = 39U,
     .density = 4.469e+0_g_cm3},
    {.canonical_name = "Zirconium",
     .atomic_number = 40U,
     .density = 6.506e+0_g_cm3},
    {.canonical_name = "Niobium",
     .atomic_number = 41U,
     .density = 8.570e+0_g_cm3},
    {.canonical_name = "Molybdenum",
     .atomic_number = 42U,
     .density = 1.022e+1_g_cm3},
    {.canonical_name = "Technetium",
     .atomic_number = 43U,
     .density = 1.150e+1_g_cm3},
    {.canonical_name = "Ruthenium",
     .atomic_number = 44U,
     .density = 1.241e+1_g_cm3},
    {.canonical_name = "Rhodium",
     .atomic_number = 45U,
     .density = 1.241e+1_g_cm3},
    {.canonical_name = "Palladium",
     .atomic_number = 46U,
     .density = 1.202e+1_g_cm3},
    {.canonical_name = "Silver",
     .atomic_number = 47U,
     .density = 1.050e+1_g_cm3},
    {.canonical_name = "Cadmium",
     .atomic_number = 48U,
     .density = 8.650e+0_g_cm3},
    {.canonical_name = "Indium",
     .atomic_number = 49U,
     .density = 7.310e+0_g_cm3},
    {.canonical_name = "Tin", .atomic_number = 50U, .density = 7.310e+0_g_cm3},
    {.canonical_name = "Antimony",
     .atomic_number = 51U,
     .density = 6.691e+0_g_cm3},
    {.canonical_name = "Tellurium",
     .atomic_number = 52U,
     .density = 6.240e+0_g_cm3},
    {.canonical_name = "Iodine",
     .atomic_number = 53U,
     .density = 4.930e+0_g_cm3},
    {.canonical_name = "Xenon",
     .atomic_number = 54U,
     .density = 5.485e-3_g_cm3},
    {.canonical_name = "Cesium",
     .atomic_number = 55U,
     .density = 1.873e+0_g_cm3},
    {.canonical_name = "Barium",
     .atomic_number = 56U,
     .density = 3.500e+0_g_cm3},
    {.canonical_name = "Lanthanum",
     .atomic_number = 57U,
     .density = 6.154e+0_g_cm3},
    {.canonical_name = "Cerium",
     .atomic_number = 58U,
     .density = 6.657e+0_g_cm3},
    {.canonical_name = "Praseodymium",
     .atomic_number = 59U,
     .density = 6.710e+0_g_cm3},
    {.canonical_name = "Neodymium",
     .atomic_number = 60U,
     .density = 6.900e+0_g_cm3},
    {.canonical_name = "Promethium",
     .atomic_number = 61U,
     .density = 7.220e+0_g_cm3},
    {.canonical_name = "Samarium",
     .atomic_number = 62U,
     .density = 7.460e+0_g_cm3},
    {.canonical_name = "Europium",
     .atomic_number = 63U,
     .density = 5.243e+0_g_cm3},
    {.canonical_name = "Gadolinium",
     .atomic_number = 64U,
     .density = 7.900e+0_g_cm3},
    {.canonical_name = "Terbium",
     .atomic_number = 65U,
     .density = 8.229e+0_g_cm3},
    {.canonical_name = "Dysprosium",
     .atomic_number = 66U,
     .density = 8.550e+0_g_cm3},
    {.canonical_name = "Holmium",
     .atomic_number = 67U,
     .density = 8.795e+0_g_cm3},
    {.canonical_name = "Erbium",
     .atomic_number = 68U,
     .density = 9.066e+0_g_cm3},
    {.canonical_name = "Thulium",
     .atomic_number = 69U,
     .density = 9.321e+0_g_cm3},
    {.canonical_name = "Ytterbium",
     .atomic_number = 70U,
     .density = 6.730e+0_g_cm3},
    {.canonical_name = "Lutetium",
     .atomic_number = 71U,
     .density = 9.840e+0_g_cm3},
    {.canonical_name = "Hafnium",
     .atomic_number = 72U,
     .density = 1.331e+1_g_cm3},
    {.canonical_name = "Tantalum",
     .atomic_number = 73U,
     .density = 1.665e+1_g_cm3},
    {.canonical_name = "Tungsten",
     .atomic_number = 74U,
     .density = 1.930e+1_g_cm3},
    {.canonical_name = "Rhenium",
     .atomic_number = 75U,
     .density = 2.102e+1_g_cm3},
    {.canonical_name = "Osmium",
     .atomic_number = 76U,
     .density = 2.257e+1_g_cm3},
    {.canonical_name = "Iridium",
     .atomic_number = 77U,
     .density = 2.242e+1_g_cm3},
    {.canonical_name = "Platinum",
     .atomic_number = 78U,
     .density = 2.145e+1_g_cm3},
    {.canonical_name = "Gold", .atomic_number = 79U, .density = 1.932e+1_g_cm3},
    {.canonical_name = "Mercury",
     .atomic_number = 80U,
     .density = 1.355e+1_g_cm3},
    {.canonical_name = "Thallium",
     .atomic_number = 81U,
     .density = 1.172e+1_g_cm3},
    {.canonical_name = "Lead", .atomic_number = 82U, .density = 1.135e+1_g_cm3},
    {.canonical_name = "Bismuth",
     .atomic_number = 83U,
     .density = 9.747e+0_g_cm3},
    {.canonical_name = "Polonium",
     .atomic_number = 84U,
     .density = 9.320e+0_g_cm3},
    {.canonical_name = "Astatine",
     .atomic_number = 85U,
     .density = 1.000e+1_g_cm3}, // NIST nominal density.
    {.canonical_name = "Radon",
     .atomic_number = 86U,
     .density = 9.066e-3_g_cm3},
    {.canonical_name = "Francium",
     .atomic_number = 87U,
     .density = 1.000e+1_g_cm3}, // NIST nominal density.
    {.canonical_name = "Radium",
     .atomic_number = 88U,
     .density = 5.000e+0_g_cm3},
    {.canonical_name = "Actinium",
     .atomic_number = 89U,
     .density = 1.007e+1_g_cm3},
    {.canonical_name = "Thorium",
     .atomic_number = 90U,
     .density = 1.172e+1_g_cm3},
    {.canonical_name = "Protactinium",
     .atomic_number = 91U,
     .density = 1.537e+1_g_cm3},
    {.canonical_name = "Uranium",
     .atomic_number = 92U,
     .density = 1.895e+1_g_cm3},
}};

// =============================================================================
// =============================================================================

constexpr std::array<Component, 4U> k_air_composition{{
    {.atomic_number = 6U, .mass_fraction = 0.000124L},
    {.atomic_number = 7U, .mass_fraction = 0.755268L},
    {.atomic_number = 8U, .mass_fraction = 0.231781L},
    {.atomic_number = 18U, .mass_fraction = 0.012827L},
}};

constexpr std::array<Component, 2U> k_water_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.111898L},
    {.atomic_number = 8U, .mass_fraction = 0.888102L},
}};

constexpr std::array<Component, 13U> k_adipose_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.119477L},
    {.atomic_number = 6U, .mass_fraction = 0.637240L},
    {.atomic_number = 7U, .mass_fraction = 0.007970L},
    {.atomic_number = 8U, .mass_fraction = 0.232333L},
    {.atomic_number = 11U, .mass_fraction = 0.000500L},
    {.atomic_number = 12U, .mass_fraction = 0.000020L},
    {.atomic_number = 15U, .mass_fraction = 0.000160L},
    {.atomic_number = 16U, .mass_fraction = 0.000730L},
    {.atomic_number = 17U, .mass_fraction = 0.001190L},
    {.atomic_number = 19U, .mass_fraction = 0.000320L},
    {.atomic_number = 20U, .mass_fraction = 0.000020L},
    {.atomic_number = 26U, .mass_fraction = 0.000020L},
    {.atomic_number = 30U, .mass_fraction = 0.000020L},
}};

constexpr std::array<Component, 10U> k_blood_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.102L},
    {.atomic_number = 6U, .mass_fraction = 0.11L},
    {.atomic_number = 7U, .mass_fraction = 0.033L},
    {.atomic_number = 8U, .mass_fraction = 0.745L},
    {.atomic_number = 11U, .mass_fraction = 0.001L},
    {.atomic_number = 15U, .mass_fraction = 0.001L},
    {.atomic_number = 16U, .mass_fraction = 0.002L},
    {.atomic_number = 17U, .mass_fraction = 0.003L},
    {.atomic_number = 19U, .mass_fraction = 0.002L},
    {.atomic_number = 26U, .mass_fraction = 0.001L},
}};

constexpr std::array<Component, 11U> k_blood_iodine5_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.0971L},
    {.atomic_number = 6U, .mass_fraction = 0.104L},
    {.atomic_number = 7U, .mass_fraction = 0.0314L},
    {.atomic_number = 8U, .mass_fraction = 0.708L},
    {.atomic_number = 11U, .mass_fraction = 0.00095L},
    {.atomic_number = 15U, .mass_fraction = 0.00095L},
    {.atomic_number = 16U, .mass_fraction = 0.0019L},
    {.atomic_number = 17U, .mass_fraction = 0.00285L},
    {.atomic_number = 19U, .mass_fraction = 0.0019L},
    {.atomic_number = 26U, .mass_fraction = 0.00095L},
    {.atomic_number = 53U, .mass_fraction = 0.05L},
}};

constexpr std::array<Component, 11U> k_blood_iodine10_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.0918L},
    {.atomic_number = 6U, .mass_fraction = 0.099L},
    {.atomic_number = 7U, .mass_fraction = 0.0297L},
    {.atomic_number = 8U, .mass_fraction = 0.6705L},
    {.atomic_number = 11U, .mass_fraction = 0.0009L},
    {.atomic_number = 15U, .mass_fraction = 0.0009L},
    {.atomic_number = 16U, .mass_fraction = 0.0018L},
    {.atomic_number = 17U, .mass_fraction = 0.0027L},
    {.atomic_number = 19U, .mass_fraction = 0.0018L},
    {.atomic_number = 26U, .mass_fraction = 0.0009L},
    {.atomic_number = 53U, .mass_fraction = 0.1L},
}};

constexpr std::array<Component, 11U> k_blood_iodine15_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.0867L},
    {.atomic_number = 6U, .mass_fraction = 0.0935L},
    {.atomic_number = 7U, .mass_fraction = 0.02805L},
    {.atomic_number = 8U, .mass_fraction = 0.63325L},
    {.atomic_number = 11U, .mass_fraction = 0.00085L},
    {.atomic_number = 15U, .mass_fraction = 0.00085L},
    {.atomic_number = 16U, .mass_fraction = 0.0017L},
    {.atomic_number = 17U, .mass_fraction = 0.00255L},
    {.atomic_number = 19U, .mass_fraction = 0.0017L},
    {.atomic_number = 26U, .mass_fraction = 0.00085L},
    {.atomic_number = 53U, .mass_fraction = 0.15L},
}};

constexpr std::array<Component, 11U> k_blood_iodine20_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.0816L},
    {.atomic_number = 6U, .mass_fraction = 0.088L},
    {.atomic_number = 7U, .mass_fraction = 0.0264L},
    {.atomic_number = 8U, .mass_fraction = 0.596L},
    {.atomic_number = 11U, .mass_fraction = 0.0008L},
    {.atomic_number = 15U, .mass_fraction = 0.0008L},
    {.atomic_number = 16U, .mass_fraction = 0.0016L},
    {.atomic_number = 17U, .mass_fraction = 0.0024L},
    {.atomic_number = 19U, .mass_fraction = 0.0016L},
    {.atomic_number = 26U, .mass_fraction = 0.0008L},
    {.atomic_number = 53U, .mass_fraction = 0.2L},
}};

constexpr std::array<Component, 13U> k_brain_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.110667L},
    {.atomic_number = 6U, .mass_fraction = 0.125420L},
    {.atomic_number = 7U, .mass_fraction = 0.013280L},
    {.atomic_number = 8U, .mass_fraction = 0.737723L},
    {.atomic_number = 11U, .mass_fraction = 0.001840L},
    {.atomic_number = 12U, .mass_fraction = 0.000150L},
    {.atomic_number = 15U, .mass_fraction = 0.003540L},
    {.atomic_number = 16U, .mass_fraction = 0.001770L},
    {.atomic_number = 17U, .mass_fraction = 0.002360L},
    {.atomic_number = 19U, .mass_fraction = 0.003100L},
    {.atomic_number = 20U, .mass_fraction = 0.000090L},
    {.atomic_number = 26U, .mass_fraction = 0.000050L},
    {.atomic_number = 30U, .mass_fraction = 0.000010L},
}};

constexpr std::array<Component, 8U> k_breast_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.1060L},
    {.atomic_number = 6U, .mass_fraction = 0.3320L},
    {.atomic_number = 7U, .mass_fraction = 0.0300L},
    {.atomic_number = 8U, .mass_fraction = 0.5270L},
    {.atomic_number = 11U, .mass_fraction = 0.0010L},
    {.atomic_number = 15U, .mass_fraction = 0.0010L},
    {.atomic_number = 16U, .mass_fraction = 0.0020L},
    {.atomic_number = 17U, .mass_fraction = 0.0010L},
}};

constexpr std::array<Component, 9U> k_heart_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.104L},
    {.atomic_number = 6U, .mass_fraction = 0.139L},
    {.atomic_number = 7U, .mass_fraction = 0.029L},
    {.atomic_number = 8U, .mass_fraction = 0.718L},
    {.atomic_number = 11U, .mass_fraction = 0.001L},
    {.atomic_number = 15U, .mass_fraction = 0.002L},
    {.atomic_number = 16U, .mass_fraction = 0.002L},
    {.atomic_number = 17U, .mass_fraction = 0.002L},
    {.atomic_number = 19U, .mass_fraction = 0.003L},
}};

constexpr std::array<Component, 9U> k_intestine_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.106L},
    {.atomic_number = 6U, .mass_fraction = 0.115L},
    {.atomic_number = 7U, .mass_fraction = 0.022L},
    {.atomic_number = 8U, .mass_fraction = 0.751L},
    {.atomic_number = 11U, .mass_fraction = 0.001L},
    {.atomic_number = 15U, .mass_fraction = 0.001L},
    {.atomic_number = 16U, .mass_fraction = 0.001L},
    {.atomic_number = 17U, .mass_fraction = 0.002L},
    {.atomic_number = 19U, .mass_fraction = 0.001L},
}};

constexpr std::array<Component, 10U> k_kidney_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.103L},
    {.atomic_number = 6U, .mass_fraction = 0.132L},
    {.atomic_number = 7U, .mass_fraction = 0.03L},
    {.atomic_number = 8U, .mass_fraction = 0.724L},
    {.atomic_number = 11U, .mass_fraction = 0.002L},
    {.atomic_number = 15U, .mass_fraction = 0.002L},
    {.atomic_number = 16U, .mass_fraction = 0.002L},
    {.atomic_number = 17U, .mass_fraction = 0.002L},
    {.atomic_number = 19U, .mass_fraction = 0.002L},
    {.atomic_number = 20U, .mass_fraction = 0.001L},
}};

constexpr std::array<Component, 9U> k_liver_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.102L},
    {.atomic_number = 6U, .mass_fraction = 0.139L},
    {.atomic_number = 7U, .mass_fraction = 0.03L},
    {.atomic_number = 8U, .mass_fraction = 0.716L},
    {.atomic_number = 11U, .mass_fraction = 0.002L},
    {.atomic_number = 15U, .mass_fraction = 0.003L},
    {.atomic_number = 16U, .mass_fraction = 0.003L},
    {.atomic_number = 17U, .mass_fraction = 0.002L},
    {.atomic_number = 19U, .mass_fraction = 0.003L},
}};

constexpr std::array<Component, 9U> k_lung_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.103L},
    {.atomic_number = 6U, .mass_fraction = 0.105L},
    {.atomic_number = 7U, .mass_fraction = 0.031L},
    {.atomic_number = 8U, .mass_fraction = 0.749L},
    {.atomic_number = 11U, .mass_fraction = 0.002L},
    {.atomic_number = 15U, .mass_fraction = 0.002L},
    {.atomic_number = 16U, .mass_fraction = 0.003L},
    {.atomic_number = 17U, .mass_fraction = 0.003L},
    {.atomic_number = 19U, .mass_fraction = 0.002L},
}};

constexpr std::array<Component, 9U> k_rib_bone_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.034L},
    {.atomic_number = 6U, .mass_fraction = 0.155L},
    {.atomic_number = 7U, .mass_fraction = 0.042L},
    {.atomic_number = 8U, .mass_fraction = 0.435L},
    {.atomic_number = 11U, .mass_fraction = 0.001L},
    {.atomic_number = 12U, .mass_fraction = 0.002L},
    {.atomic_number = 15U, .mass_fraction = 0.103L},
    {.atomic_number = 16U, .mass_fraction = 0.003L},
    {.atomic_number = 20U, .mass_fraction = 0.225L},
}};

constexpr std::array<Component, 11U> k_spine_bone_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.063L},
    {.atomic_number = 6U, .mass_fraction = 0.261L},
    {.atomic_number = 7U, .mass_fraction = 0.039L},
    {.atomic_number = 8U, .mass_fraction = 0.436L},
    {.atomic_number = 11U, .mass_fraction = 0.001L},
    {.atomic_number = 12U, .mass_fraction = 0.001L},
    {.atomic_number = 15U, .mass_fraction = 0.061L},
    {.atomic_number = 16U, .mass_fraction = 0.003L},
    {.atomic_number = 17U, .mass_fraction = 0.001L},
    {.atomic_number = 19U, .mass_fraction = 0.001L},
    {.atomic_number = 20U, .mass_fraction = 0.133L},
}};

constexpr std::array<Component, 9U> k_spleen_composition{{
    {.atomic_number = 1U, .mass_fraction = 0.103L},
    {.atomic_number = 6U, .mass_fraction = 0.113L},
    {.atomic_number = 7U, .mass_fraction = 0.032L},
    {.atomic_number = 8U, .mass_fraction = 0.741L},
    {.atomic_number = 11U, .mass_fraction = 0.001L},
    {.atomic_number = 15U, .mass_fraction = 0.003L},
    {.atomic_number = 16U, .mass_fraction = 0.002L},
    {.atomic_number = 17U, .mass_fraction = 0.002L},
    {.atomic_number = 19U, .mass_fraction = 0.003L},
}};

constexpr std::array<Component, 2U> k_cdte_composition{{
    {.atomic_number = 48U, .mass_fraction = 0.468358L},
    {.atomic_number = 52U, .mass_fraction = 0.531642L},
}};

constexpr std::array<Component, 2U> k_csi_composition{{
    {.atomic_number = 53U, .mass_fraction = 0.488451L},
    {.atomic_number = 55U, .mass_fraction = 0.511549L},
}};

constexpr std::array<Component, 2U> k_gaas_composition{{
    {.atomic_number = 31U, .mass_fraction = 0.482030L},
    {.atomic_number = 33U, .mass_fraction = 0.517970L},
}};

constexpr std::array<Component, 3U> k_gos_composition{{
    {.atomic_number = 8U, .mass_fraction = 0.084527L},
    {.atomic_number = 16U, .mass_fraction = 0.084704L},
    {.atomic_number = 64U, .mass_fraction = 0.830769L},
}};

constexpr std::array<Component, 3U> k_lso_composition{{
    {.atomic_number = 8U, .mass_fraction = 0.174L},
    {.atomic_number = 14U, .mass_fraction = 0.062L},
    {.atomic_number = 71U, .mass_fraction = 0.764L},
}};

constexpr std::array<Component, 2U> k_nai_composition{{
    {.atomic_number = 11U, .mass_fraction = 0.153L},
    {.atomic_number = 53U, .mass_fraction = 0.847L},
}};

// =============================================================================
// =============================================================================

constexpr std::array<CompoundMaterialRow, 24U> k_compound_materials{
    {// Common media.
     {.canonical_name = "Air",
      .density = 1.205e-3_g_cm3,
      .composition = std::span<Component const>{k_air_composition}},
     {.canonical_name = "Water",
      .density = 1.000_g_cm3,
      .composition = std::span<Component const>{k_water_composition}},
     // Medical tissues.
     {.canonical_name = "Adipose",
      .density = 0.92_g_cm3,
      .composition = std::span<Component const>{k_adipose_composition}},
     {.canonical_name = "Blood",
      .density = 1.06_g_cm3,
      .composition = std::span<Component const>{k_blood_composition}},
     {.canonical_name = "BloodIodine5",
      .density = 1.25_g_cm3,
      .composition = std::span<Component const>{k_blood_iodine5_composition}},
     {.canonical_name = "BloodIodine10",
      .density = 1.44_g_cm3,
      .composition = std::span<Component const>{k_blood_iodine10_composition}},
     {.canonical_name = "BloodIodine15",
      .density = 1.64_g_cm3,
      .composition = std::span<Component const>{k_blood_iodine15_composition}},
     {.canonical_name = "BloodIodine20",
      .density = 1.834_g_cm3,
      .composition = std::span<Component const>{k_blood_iodine20_composition}},
     {.canonical_name = "Brain",
      .density = 1.03_g_cm3,
      .composition = std::span<Component const>{k_brain_composition}},
     {.canonical_name = "Breast",
      .density = 1.020_g_cm3,
      .composition = std::span<Component const>{k_breast_composition}},
     {.canonical_name = "Heart",
      .density = 1.05_g_cm3,
      .composition = std::span<Component const>{k_heart_composition}},
     {.canonical_name = "Intestine",
      .density = 1.03_g_cm3,
      .composition = std::span<Component const>{k_intestine_composition}},
     {.canonical_name = "Kidney",
      .density = 1.05_g_cm3,
      .composition = std::span<Component const>{k_kidney_composition}},
     {.canonical_name = "Liver",
      .density = 1.06_g_cm3,
      .composition = std::span<Component const>{k_liver_composition}},
     {.canonical_name = "Lung",
      .density = 0.26_g_cm3,
      .composition = std::span<Component const>{k_lung_composition}},
     {.canonical_name = "RibBone",
      .density = 1.92_g_cm3,
      .composition = std::span<Component const>{k_rib_bone_composition}},
     {.canonical_name = "SpineBone",
      .density = 1.42_g_cm3,
      .composition = std::span<Component const>{k_spine_bone_composition}},
     {.canonical_name = "Spleen",
      .density = 1.06_g_cm3,
      .composition = std::span<Component const>{k_spleen_composition}},
     // Detector materials.
     {.canonical_name = "CdTe",
      .density = 6.200_g_cm3,
      .composition = std::span<Component const>{k_cdte_composition}},
     {.canonical_name = "CsI",
      .density = 4.510_g_cm3,
      .composition = std::span<Component const>{k_csi_composition}},
     {.canonical_name = "GaAs",
      .density = 5.310_g_cm3,
      .composition = std::span<Component const>{k_gaas_composition}},
     {.canonical_name = "GOS",
      .density = 7.440_g_cm3,
      .composition = std::span<Component const>{k_gos_composition}},
     {.canonical_name = "LSO",
      .density = 7.4_g_cm3,
      .composition = std::span<Component const>{k_lso_composition}},
     {.canonical_name = "NaI",
      .density = 3.67_g_cm3,
      .composition = std::span<Component const>{k_nai_composition}}}};

// =============================================================================
// =============================================================================

constexpr std::size_t k_builtin_material_count =
    1U + k_elemental_materials.size() + k_compound_materials.size();

// =============================================================================
// =============================================================================

[[nodiscard]] consteval auto BuildAvailableMaterialNames()
    -> std::array<std::string_view, k_builtin_material_count> {
  std::array<std::string_view, k_builtin_material_count> names{};
  std::size_t index{0U};

  names[index++] = "Vacuum";

  for (auto const &row : k_elemental_materials) {
    names[index++] = row.canonical_name;
  }

  for (auto const &row : k_compound_materials) {
    names[index++] = row.canonical_name;
  }

  return names;
}

// =============================================================================
// =============================================================================

constexpr auto k_available_material_names = BuildAvailableMaterialNames();

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildElementalMaterial(ElementalMaterialRow const &row)
    -> GGEMSMaterial {
  return GGEMSMaterial{
      std::string{row.canonical_name},
      row.density,
      {{.atomic_number = row.atomic_number, .mass_fraction = 1.0L}}};
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildCompoundMaterial(CompoundMaterialRow const &row)
    -> GGEMSMaterial {
  return GGEMSMaterial{
      std::string{row.canonical_name}, row.density,
      std::vector<Component>{row.composition.begin(), row.composition.end()}};
}

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetAvailableMaterialNames() noexcept
    -> std::span<std::string_view const> {
  return k_available_material_names;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildBuiltInMaterial(std::string_view canonical_name)
    -> GGEMSMaterial {
  if (canonical_name == "Vacuum") {
    return GGEMSMaterial{"Vacuum", 0.0_g_cm3, {}};
  }

  for (auto const &row : k_elemental_materials) {
    if (row.canonical_name == canonical_name) {
      return BuildElementalMaterial(row);
    }
  }

  for (auto const &row : k_compound_materials) {
    if (row.canonical_name == canonical_name) {
      return BuildCompoundMaterial(row);
    }
  }

  throw GGEMSRecoverable{
      std::format("Unknown built-in Material '{}'.", canonical_name)};
}

} // namespace ggems::core::materials::builtins
