#include <optional>
#include <string_view>
#include <array>
#include <cstddef>
#include <cstdint>
#include <format>
#include <span>
#include <string>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/logging/GGEMSLogMacros.hh"
#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"
#include "GGEMS/radioactivity/GGEMSRadionuclideEmission.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"
#include "GGEMS/sources/GGEMSSourceTypes.hh"
#include "GGEMS/units/GGEMSEnergyUnits.hh"
#include "GGEMS/units/GGEMSUnitFormatting.hh"

namespace ggems::core::radioactivity::builtins {

namespace {

// =============================================================================
// =============================================================================

using Builder = GGEMSRadionuclideDefinition (*)();

// =============================================================================
// =============================================================================

constexpr GGEMSBuiltInRadionuclideInfo k_h3_info{
    .canonical_name = "H-3",
    .element_name = "Hydrogen",
    .atomic_number = 1U,
    .mass_number = 3U,
    .daughter_name = "He-3",
    .decay_mode = "beta-",
    .q_value_kilo_electron_volt = 18.591L,
    .nuclear_data_source = "LNHB / DDEP 2006",
    .beta_spectrum_source = "BetaShape 2.2 (experimental shape factor)",
    .atomic_data_source = "Not used by current GGEMS definition",
    .model_notes =
        "BetaShape uses its supplied 18.591 keV energy axis; GGEMS does not "
        "rescale it to the separate 18.564 keV atomic endpoint."};

constexpr GGEMSBuiltInRadionuclideInfo k_c14_info{
    .canonical_name = "C-14",
    .element_name = "Carbon",
    .atomic_number = 6U,
    .mass_number = 14U,
    .daughter_name = "N-14",
    .decay_mode = "beta-",
    .q_value_kilo_electron_volt = 156.476L,
    .nuclear_data_source = "LNHB / DDEP 2012",
    .beta_spectrum_source = "BetaShape 2.2 (experimental shape factor)",
    .atomic_data_source = "Not used by current GGEMS definition",
    .model_notes =
        "The measured non-allowed beta spectral shape is used; no prompt "
        "gamma or atomic-relaxation emission is present in the selected "
        "decay model."};

constexpr GGEMSBuiltInRadionuclideInfo k_f18_info{
    .canonical_name = "F-18",
    .element_name = "Fluorine",
    .atomic_number = 9U,
    .mass_number = 18U,
    .daughter_name = "O-18",
    .decay_mode = "beta+ / electron capture",
    .q_value_kilo_electron_volt = 1'655.9L,
    .nuclear_data_source = "LNHB / DDEP",
    .beta_spectrum_source = "BetaShape 2.2 (experimental shape factor)",
    .atomic_data_source = "LNHB / DDEP",
    .model_notes =
        "No source 511 keV annihilation photons; electron capture has no "
        "placeholder emission."};

constexpr GGEMSBuiltInRadionuclideInfo k_c11_info{
    .canonical_name = "C-11",
    .element_name = "Carbon",
    .atomic_number = 6U,
    .mass_number = 11U,
    .daughter_name = "B-11",
    .decay_mode = "beta+ / electron capture",
    .q_value_kilo_electron_volt = 1'982.5L,
    .nuclear_data_source = "LNHB / DDEP",
    .beta_spectrum_source = "BetaShape 2.2 (experimental shape factor)",
    .atomic_data_source = "Not used by current GGEMS definition",
    .model_notes =
        "No source 511 keV annihilation photons; the electron-capture branch "
        "has no placeholder or atomic-relaxation emission in the current "
        "GGEMS definition."};

constexpr GGEMSBuiltInRadionuclideInfo k_o15_info{
    .canonical_name = "O-15",
    .element_name = "Oxygen",
    .atomic_number = 8U,
    .mass_number = 15U,
    .daughter_name = "N-15",
    .decay_mode = "beta+ / electron capture",
    .q_value_kilo_electron_volt = 2'754.18L,
    .nuclear_data_source = "LNHB / DDEP",
    .beta_spectrum_source = "BetaShape 2.2 (calculated; energy-axis rescaled)",
    .atomic_data_source = "Not used by current GGEMS definition",
    .model_notes =
        "The BetaShape energy axis is rescaled from 1735.0 keV to the "
        "selected 1732.18 keV positron endpoint. No source 511 keV "
        "annihilation photons are emitted."};

constexpr GGEMSBuiltInRadionuclideInfo k_ga68_info{
    .canonical_name = "Ga-68",
    .element_name = "Gallium",
    .atomic_number = 31U,
    .mass_number = 68U,
    .daughter_name = "Zn-68",
    .decay_mode = "beta+ / electron capture",
    .q_value_kilo_electron_volt = 2'921.1L,
    .nuclear_data_source = "LNHB / PTB / DDEP",
    .beta_spectrum_source = "BetaShape 2.2 (experimental + calculated)",
    .atomic_data_source =
        "LNHB / PTB / DDEP + MIRDspecs / ICRP 107 (Auger only)",
    .model_notes =
        "No source 511 keV annihilation photons. Electron-capture branches "
        "have no placeholder primary; evaluated atomic relaxations are kept. "
        "The 1655.87 keV E0 transition is not fabricated as a gamma."};

constexpr GGEMSBuiltInRadionuclideInfo k_co60_info{
    .canonical_name = "Co-60",
    .element_name = "Cobalt",
    .atomic_number = 27U,
    .mass_number = 60U,
    .daughter_name = "Ni-60",
    .decay_mode = "beta-",
    .q_value_kilo_electron_volt = 2'823.07L,
    .nuclear_data_source = "LNHB / DDEP",
    .beta_spectrum_source = "BetaShape 2.2 (experimental + calculated)",
    .atomic_data_source = "LNHB / DDEP + MIRDspecs / ICRP 107 (Auger only)",
    .model_notes =
        "Tiny internal-pair formation on the 1173 and 1332 keV transitions "
        "is not modelled because the current flattened emission model cannot "
        "represent correlated electron-positron energy sharing."};

constexpr GGEMSBuiltInRadionuclideInfo k_lu177_info{
    .canonical_name = "Lu-177",
    .element_name = "Lutetium",
    .atomic_number = 71U,
    .mass_number = 177U,
    .daughter_name = "Hf-177",
    .decay_mode = "beta-",
    .q_value_kilo_electron_volt = 496.8L,
    .nuclear_data_source = "LNHB / DDEP 2025",
    .beta_spectrum_source = "BetaShape 2.2 (calculated; energy-axis rescaled)",
    .atomic_data_source =
        "LNHB / DDEP 2025 + MIRDspecs / ICRP 107 (Auger only)",
    .model_notes =
        "The BetaShape transition files predate the 2025 LNHB evaluation; "
        "their energy axes are rescaled to the current evaluated branch "
        "endpoints while preserving the conditional spectral shapes."};

constexpr GGEMSBuiltInRadionuclideInfo k_i131_info{
    .canonical_name = "I-131",
    .element_name = "Iodine",
    .atomic_number = 53U,
    .mass_number = 131U,
    .daughter_name = "Xe-131",
    .decay_mode = "beta-",
    .q_value_kilo_electron_volt = 970.8L,
    .nuclear_data_source = "LNHB / DDEP",
    .beta_spectrum_source = "BetaShape 2.2 (experimental + calculated)",
    .atomic_data_source = "LNHB / DDEP + MIRDspecs / ICRP 107 (Auger only)",
    .model_notes =
        "The beta branch populating Xe-131m is included, but the delayed "
        "163.930 keV Xe-131m transition and its conversion electrons are not "
        "flattened into the parent I-131 decay."};

constexpr GGEMSBuiltInRadionuclideInfo k_am241_info{
    .canonical_name = "Am-241",
    .element_name = "Americium",
    .atomic_number = 95U,
    .mass_number = 241U,
    .daughter_name = "Np-237",
    .decay_mode = "alpha",
    .q_value_kilo_electron_volt = 5'637.82L,
    .nuclear_data_source = "LNHB / DDEP",
    .beta_spectrum_source = "Not applicable",
    .atomic_data_source = "LNHB / DDEP + MIRDspecs / ICRP 107 (Auger only)",
    .model_notes =
        "Daughter-chain emissions and alpha-recoil nuclei are not included. "
        "The 516 conversion-electron lines are split into two numerical "
        "transport groups without dropping or renormalizing any line."};

constexpr GGEMSBuiltInRadionuclideInfo k_tc99m_info{
    .canonical_name = "Tc-99m",
    .element_name = "Technetium",
    .atomic_number = 43U,
    .mass_number = 99U,
    .daughter_name = "Tc-99 / Ru-99",
    .decay_mode = "internal transition / beta-",
    .q_value_kilo_electron_volt = 436.3L,
    .nuclear_data_source = "LNHB / DDEP",
    .beta_spectrum_source = "BetaShape 2.2 (calculated; energy-axis rescaled)",
    .atomic_data_source = "LNHB / DDEP + MIRDspecs / ICRP 107 (Auger only)",
    .model_notes =
        "The Q value shown is the maximum rare beta-minus branch energy to "
        "Ru-99; the dominant isomeric transition is about 142.684 keV. Rare "
        "direct Ru-99 emissions are included, but later Tc-99 decay is not "
        "tracked."};

// =============================================================================
// =============================================================================

struct BuiltInEntry {
  std::string_view canonical_name;
  Builder builder;
  GGEMSBuiltInRadionuclideInfo const *info;
};

// =============================================================================
// =============================================================================

constexpr std::array<BuiltInEntry, 11U> k_builtin_entries{{
    {.canonical_name = "H-3",
     .builder = BuildH3Radionuclide,
     .info = &k_h3_info},
    {.canonical_name = "C-14",
     .builder = BuildC14Radionuclide,
     .info = &k_c14_info},
    {.canonical_name = "F-18",
     .builder = BuildF18Radionuclide,
     .info = &k_f18_info},
    {.canonical_name = "C-11",
     .builder = BuildC11Radionuclide,
     .info = &k_c11_info},
    {.canonical_name = "O-15",
     .builder = BuildO15Radionuclide,
     .info = &k_o15_info},
    {.canonical_name = "Ga-68",
     .builder = BuildGa68Radionuclide,
     .info = &k_ga68_info},
    {.canonical_name = "Co-60",
     .builder = BuildCo60Radionuclide,
     .info = &k_co60_info},
    {.canonical_name = "Lu-177",
     .builder = BuildLu177Radionuclide,
     .info = &k_lu177_info},
    {.canonical_name = "I-131",
     .builder = BuildI131Radionuclide,
     .info = &k_i131_info},
    {.canonical_name = "Am-241",
     .builder = BuildAm241Radionuclide,
     .info = &k_am241_info},
    {.canonical_name = "Tc-99m",
     .builder = BuildTc99mRadionuclide,
     .info = &k_tc99m_info},
}};

// =============================================================================
// =============================================================================

[[nodiscard]] consteval auto BuildAvailableRadionuclideNames()
    -> std::array<std::string_view, k_builtin_entries.size()> {
  std::array<std::string_view, k_builtin_entries.size()> names{};
  for (std::size_t index = 0U; index < k_builtin_entries.size(); ++index) {
    names[index] = k_builtin_entries[index].canonical_name;
  }
  return names;
}

// =============================================================================
// =============================================================================

constexpr auto k_available_radionuclide_names =
    BuildAvailableRadionuclideNames();

// =============================================================================
// =============================================================================

[[nodiscard]] auto
DescribeEnergyDistribution(sources::GGEMSEnergyDistribution const &distribution)
    -> std::string {
  sources::GGEMSEnergyDistributionType const type = distribution.GetType();

  if (type == sources::GGEMSEnergyDistributionType::Mono) {
    return std::format("Mono {}",
                       ggems::units::HumanReadable(ggems::units::Energy{
                           distribution.GetMonoEnergyMilliElectronVolt()}));
  }

  auto const energies = distribution.GetEnergyValuesMilliElectronVolt();

  if (energies.empty()) {
    throw ggems::core::GGEMSInternal(
        "Built-in radionuclide table-backed energy distribution is empty.");
  }

  if (type == sources::GGEMSEnergyDistributionType::DiscreteLines) {
    return std::format(
        "Discrete lines | {} lines | range [{}, {}]", energies.size(),
        ggems::units::HumanReadable(ggems::units::Energy{energies.front()}),
        ggems::units::HumanReadable(ggems::units::Energy{energies.back()}));
  }

  if (type == sources::GGEMSEnergyDistributionType::RegularSpectrum) {
    std::uint64_t const bin_width =
        distribution.GetRegularBinWidthMilliElectronVolt();
    std::uint64_t const endpoint = energies.back() + (bin_width / 2ULL);
    return std::format(
        "Regular spectrum | {} bins | endpoint {}", energies.size(),
        ggems::units::HumanReadable(ggems::units::Energy{endpoint}));
  }

  throw ggems::core::GGEMSInternal(
      "Unsupported built-in radionuclide energy distribution type.");
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto DescribeEmission(std::size_t index,
                                    GGEMSRadionuclideEmission const &emission)
    -> std::string {
  return std::format(
      "    [{}] {} | yield {:.8g} | {}", index,
      particles::ToLongName(emission.GetParticleType()),
      static_cast<double>(emission.GetYieldPerDecay()),
      DescribeEnergyDistribution(emission.GetEnergyDistribution()));
}

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto BuildBuiltInRadionuclide(std::string_view canonical_name)
    -> std::optional<GGEMSRadionuclideDefinition> {
  for (BuiltInEntry const &entry : k_builtin_entries) {
    if (canonical_name == entry.canonical_name) {
      return entry.builder();
    }
  }

  return std::nullopt;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetAvailableRadionuclideNames() noexcept
    -> std::span<std::string_view const> {
  return k_available_radionuclide_names;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
FindBuiltInRadionuclideInfo(std::string_view canonical_name) noexcept
    -> GGEMSBuiltInRadionuclideInfo const * {
  for (BuiltInEntry const &entry : k_builtin_entries) {
    if (canonical_name == entry.canonical_name) {
      return entry.info;
    }
  }

  return nullptr;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
DescribeBuiltInRadionuclide(GGEMSRadionuclideDefinition const &definition)
    -> std::string {
  GGEMSBuiltInRadionuclideInfo const *const info =
      FindBuiltInRadionuclideInfo(definition.GetCanonicalName());

  std::string description;
  if (info != nullptr) {
    description = std::format(
        "{}\n"
        "  Element        : {}\n"
        "  Z / A          : {} / {}\n"
        "  Daughter       : {}\n"
        "  Decay mode     : {}\n"
        "  Q value        : {:.8g} keV\n",
        info->canonical_name, info->element_name, info->atomic_number,
        info->mass_number, info->daughter_name, info->decay_mode,
        static_cast<double>(info->q_value_kilo_electron_volt));
  } else {
    description = std::format(
        "{}\n  Catalog details: not populated in the current prototype\n",
        definition.GetCanonicalName());
  }

  description +=
      std::format("  Half-life      : {:.8g} s\n"
                  "  GGEMS model    : flattened radioactive emission model\n"
                  "  Decay history  : not tracked\n"
                  "  Emissions      : {} flattened emission channels\n",
                  static_cast<double>(definition.GetHalfLifeSeconds()),
                  definition.GetEmissions().size());

  auto const emissions = definition.GetEmissions();
  for (std::size_t index = 0U; index < emissions.size(); ++index) {
    description += DescribeEmission(index, emissions[index]);
    description.push_back('\n');
  }

  description +=
      std::format("  Total yield    : {:.8g} particles/decay",
                  static_cast<double>(definition.GetTotalYieldPerDecay()));

  if (info != nullptr) {
    description +=
        std::format("\n\n  Data sources\n"
                    "    Nuclear data : {}\n"
                    "    Beta spectrum: {}\n"
                    "    Atomic data  : {}",
                    info->nuclear_data_source, info->beta_spectrum_source,
                    info->atomic_data_source);

    if (!info->model_notes.empty()) {
      description +=
          std::format("\n\n  Model notes\n    {}", info->model_notes);
    }
  }

  return description;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto DescribeBuiltInRadionuclide(std::string_view canonical_name)
    -> std::string {
  auto definition = BuildBuiltInRadionuclide(canonical_name);
  if (!definition.has_value()) {
    throw ggems::core::GGEMSRecoverable(
        std::format("Unknown built-in radionuclide '{}'.", canonical_name));
  }

  return DescribeBuiltInRadionuclide(*definition);
}

// =============================================================================
// =============================================================================

auto VerboseBuiltInRadionuclide(GGEMSRadionuclideDefinition const &definition)
    -> void {
  GGEMS_INFO("Radionuclide", "{}", DescribeBuiltInRadionuclide(definition));
}

// =============================================================================
// =============================================================================

auto VerboseBuiltInRadionuclide(std::string_view canonical_name) -> void {
  GGEMS_INFO("Radionuclide", "{}", DescribeBuiltInRadionuclide(canonical_name));
}

} // namespace ggems::core::radioactivity::builtins
