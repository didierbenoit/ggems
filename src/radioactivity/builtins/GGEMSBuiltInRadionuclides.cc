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

struct BuiltInEntry {
  std::string_view canonical_name;
  Builder builder;
};

// =============================================================================
// =============================================================================

constexpr std::array<BuiltInEntry, 14U> k_builtin_entries{
    {{.canonical_name = "H-3", .builder = BuildH3Radionuclide},
     {.canonical_name = "C-14", .builder = BuildC14Radionuclide},
     {.canonical_name = "F-18", .builder = BuildF18Radionuclide},
     {.canonical_name = "C-11", .builder = BuildC11Radionuclide},
     {.canonical_name = "O-15", .builder = BuildO15Radionuclide},
     {.canonical_name = "Ga-68", .builder = BuildGa68Radionuclide},
     {.canonical_name = "Co-60", .builder = BuildCo60Radionuclide},
     {.canonical_name = "Lu-177", .builder = BuildLu177Radionuclide},
     {.canonical_name = "I-123", .builder = BuildI123Radionuclide},
     {.canonical_name = "I-124", .builder = BuildI124Radionuclide},
     {.canonical_name = "I-125", .builder = BuildI125Radionuclide},
     {.canonical_name = "I-131", .builder = BuildI131Radionuclide},
     {.canonical_name = "Am-241", .builder = BuildAm241Radionuclide},
     {.canonical_name = "Tc-99m", .builder = BuildTc99mRadionuclide}}};

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
    auto const weights = distribution.GetRelativeWeights();

    std::size_t strongest_index{0U};
    for (std::size_t index = 1U; index < weights.size(); ++index) {
      if (weights[index] > weights[strongest_index]) {
        strongest_index = index;
      }
    }

    return std::format(
        "Discrete lines | {} lines | range [{}, {}] | strongest line {}",
        energies.size(),
        ggems::units::HumanReadable(ggems::units::Energy{energies.front()}),
        ggems::units::HumanReadable(ggems::units::Energy{energies.back()}),
        ggems::units::HumanReadable(
            ggems::units::Energy{energies[strongest_index]}));
  }

  if (type == sources::GGEMSEnergyDistributionType::RegularSpectrum) {
    std::uint64_t const bin_width =
        distribution.GetRegularBinWidthMilliElectronVolt();
    std::uint64_t const half_width = bin_width / 2ULL;
    std::uint64_t const lower_edge = energies.front() - half_width;
    std::uint64_t const upper_edge = energies.back() + half_width;

    return std::format(
        "Regular spectrum | {} bins | range [{}, {}] | bin width {}",
        energies.size(),
        ggems::units::HumanReadable(ggems::units::Energy{lower_edge}),
        ggems::units::HumanReadable(ggems::units::Energy{upper_edge}),
        ggems::units::HumanReadable(ggems::units::Energy{bin_width}));
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
DescribeBuiltInRadionuclide(GGEMSRadionuclideDefinition const &definition)
    -> std::string {
  auto const emissions = definition.GetEmissions();

  std::string description = std::format(
      "{}\n"
      "  Half-life      : {:.8g} s\n"
      "  Emissions      : {}\n",
      definition.GetCanonicalName(),
      static_cast<double>(definition.GetHalfLifeSeconds()), emissions.size());

  for (std::size_t index = 0U; index < emissions.size(); ++index) {
    description += DescribeEmission(index, emissions[index]);
    description.push_back('\n');
  }

  description +=
      std::format("  Total yield    : {:.8g} particles/decay",
                  static_cast<double>(definition.GetTotalYieldPerDecay()));

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
