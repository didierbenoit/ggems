// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Registers and describes compiled radioactive source definitions.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

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

/*! \brief Builds an owned definition from compiled source data. */
using Builder = GGEMSRadionuclideDefinition (*)();

// =============================================================================
// =============================================================================

/*! \brief Associates an exact canonical label with its catalog builder. */
struct BuiltInEntry {
  /*! \brief Canonical label backed by a string literal. */
  std::string_view canonical_name;

  /*! \brief Factory for this catalog entry. */
  Builder builder;
};

// =============================================================================
// =============================================================================

/*! \brief Canonical catalog order and owning definition factories. */
constexpr auto k_builtin_entries = std::to_array<BuiltInEntry>({
  {.canonical_name = "H-3", .builder = BuildH3Radionuclide},
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
  {.canonical_name = "Tc-99m", .builder = BuildTc99mRadionuclide},
  {.canonical_name = "P-32", .builder = BuildP32Radionuclide},
  {.canonical_name = "P-33", .builder = BuildP33Radionuclide},
  {.canonical_name = "Co-57", .builder = BuildCo57Radionuclide},
  {.canonical_name = "Ga-67", .builder = BuildGa67Radionuclide},
  {.canonical_name = "Cu-67", .builder = BuildCu67Radionuclide},
  {.canonical_name = "Sc-44", .builder = BuildSc44Radionuclide},
  {.canonical_name = "Sc-47", .builder = BuildSc47Radionuclide},
  {.canonical_name = "Mn-52", .builder = BuildMn52Radionuclide},
  {.canonical_name = "Br-76", .builder = BuildBr76Radionuclide},
  {.canonical_name = "Na-24", .builder = BuildNa24Radionuclide},
  {.canonical_name = "S-35", .builder = BuildS35Radionuclide},
  {.canonical_name = "Ca-45", .builder = BuildCa45Radionuclide},
  {.canonical_name = "Hg-203", .builder = BuildHg203Radionuclide},
  {.canonical_name = "Tl-201", .builder = BuildTl201Radionuclide},
  {.canonical_name = "Xe-133", .builder = BuildXe133Radionuclide},
  {.canonical_name = "N-13", .builder = BuildN13Radionuclide},
  {.canonical_name = "Rb-82", .builder = BuildRb82Radionuclide},
  {.canonical_name = "In-111", .builder = BuildIn111Radionuclide},
});

// =============================================================================
// =============================================================================

/*!
 * \brief Projects the static catalog into its ordered label array.
 *
 * \return Canonical names in the same order as the builder registry.
 */
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

/*! \brief Static labels exposed by the catalog enumeration API. */
constexpr auto k_available_radionuclide_names =
  BuildAvailableRadionuclideNames();

// =============================================================================
// =============================================================================

/*!
 * \brief Formats energy support and table shape for an emission law.
 *
 * \param[in] distribution Valid prepared energy law.
 * \return Monoenergy, line summary, or spectrum-bin summary.
 * \throws GGEMSInternal If the energy-law kind is unsupported.
 */
[[nodiscard]] auto
DescribeEnergyDistribution(sources::GGEMSEnergyDistribution const &distribution)
  -> std::string {
  sources::GGEMSEnergyDistributionType const type = distribution.GetType();

  if (type == sources::GGEMSEnergyDistributionType::Mono) {
    return std::format("Mono {}",
                       ggems::units::HumanReadable(ggems::units::Energy{
                         distribution.GetMonoEnergyMicroElectronVolt(),
                       }));
  }

  auto const energies = distribution.GetEnergyValuesMicroElectronVolt();

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
      distribution.GetRegularBinWidthMicroElectronVolt();
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

/*!
 * \brief Formats one indexed marginal emission description.
 *
 * \param[in] index Zero-based group index in the definition.
 * \param[in] emission Emission whose kind, yield, and energy law are described.
 * \return One indented description line.
 */
[[nodiscard]] auto DescribeEmission(std::size_t index,
                                    GGEMSRadionuclideEmission const &emission)
  -> std::string {
  return std::format(
    "    [{}] {} | yield {:.8g} | {}", index,
    particles::ToLongName(emission.GetParticleType()),
    emission.GetYieldPerDecay(),
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

  std::string description =
    std::format("{}\n"
                "  Half-life      : {:.8g} s\n"
                "  Emissions      : {}\n",
                definition.GetCanonicalName(), definition.GetHalfLifeSeconds(),
                emissions.size());

  for (std::size_t index = 0U; index < emissions.size(); ++index) {
    description += DescribeEmission(index, emissions[index]);
    description.push_back('\n');
  }

  description += std::format("  Total yield    : {:.8g} particles/decay",
                             definition.GetTotalYieldPerDecay());

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
  GGEMS_INFO("Radionuclide", "\n{}\n", DescribeBuiltInRadionuclide(definition));
}

// =============================================================================
// =============================================================================

auto VerboseBuiltInRadionuclide(std::string_view canonical_name) -> void {
  GGEMS_INFO("Radionuclide", "\n{}\n",
             DescribeBuiltInRadionuclide(canonical_name));
}

} // namespace ggems::core::radioactivity::builtins
