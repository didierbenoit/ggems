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
 * \brief Declares four-channel production-cut lengths and per-channel scope
 * resolution.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string_view>
#include <vector>
/// \endcond

#include "GGEMS/units/GGEMSLengthUnits.hh"

/*!
 * \namespace ggems::core::processes
 * \brief Prepares host production-cut thresholds and material/cut-couple data.
 */
namespace ggems::core::processes {

/*!
 * \brief Identifies the four secondary-production cut channels.
 *
 * These channels describe secondary-production policy, not termination
 * thresholds for existing tracks. There is no Neutron or Alpha channel.
 */
enum class GGEMSProductionCutChannel : std::uint8_t {
  /*! \brief Gamma secondary-production channel. */
  Gamma = 0U,

  /*! \brief Electron secondary-production channel. */
  Electron = 1U,

  /*! \brief Positron secondary-production channel. */
  Positron = 2U,

  /*! \brief Proton/recoil production convention. */
  Proton = 3U,
};

/*! \brief Defines the common Gamma, Electron, Positron, Proton array order. */
inline constexpr std::array<GGEMSProductionCutChannel, 4U>
  k_production_cut_channels{
    GGEMSProductionCutChannel::Gamma,
    GGEMSProductionCutChannel::Electron,
    GGEMSProductionCutChannel::Positron,
    GGEMSProductionCutChannel::Proton,
};

/*!
 * \brief Converts a channel to its position in four-channel arrays.
 *
 * \pre channel must be a declared enumerator when the result indexes an array.
 *
 * \param[in] channel One of the four declared channel enumerators.
 * \return The underlying index from zero through three.
 */
[[nodiscard]] constexpr auto
ProductionCutChannelIndex(GGEMSProductionCutChannel channel) noexcept
  -> std::size_t {
  return static_cast<std::size_t>(channel);
}

/*!
 * \brief Returns the diagnostic name of a cut channel.
 *
 * \param[in] channel Channel value to display.
 * \return A static string view, or Unknown for an unrecognized value.
 */
[[nodiscard]] constexpr auto
ProductionCutChannelName(GGEMSProductionCutChannel channel) noexcept
  -> std::string_view {
  switch (channel) {
  case GGEMSProductionCutChannel::Gamma:
    return "Gamma";
  case GGEMSProductionCutChannel::Electron:
    return "Electron";
  case GGEMSProductionCutChannel::Positron:
    return "Positron";
  case GGEMSProductionCutChannel::Proton:
    return "Proton";
  }
  return "Unknown";
}

/*!
 * \brief Identifies the scope that supplied a resolved production-cut length.
 */
enum class GGEMSProductionCutScope : std::uint8_t {
  /*! \brief Lowest-precedence default. */
  Global = 0U,

  /*! \brief Override for one authoring material index. */
  Material = 1U,

  /*! \brief Highest-precedence context override. */
  Volume = 2U,
};

/*!
 * \brief Returns the diagnostic name of a cut scope.
 *
 * \param[in] scope Scope value to display.
 * \return A static string view, or Unknown for an unrecognized value.
 */
[[nodiscard]] constexpr auto
ProductionCutScopeName(GGEMSProductionCutScope scope) noexcept
  -> std::string_view {
  switch (scope) {
  case GGEMSProductionCutScope::Global:
    return "Global";
  case GGEMSProductionCutScope::Material:
    return "Material";
  case GGEMSProductionCutScope::Volume:
    return "Volume";
  }
  return "Unknown";
}

/*!
 * \brief Stores optional lengths independently for the four cut channels.
 *
 * Lengths use canonical integer picometers. An absent override inherits from
 * the next scope; an absent SetProductionCuts() argument leaves global state
 * unchanged. A present zero length is a value, not an inheritance marker.
 */
struct GGEMSProductionCutLengths {
  /*! \brief Optional Gamma length in pm. */
  std::optional<units::Length> gamma;

  /*! \brief Optional Electron length in pm. */
  std::optional<units::Length> electron;

  /*! \brief Optional Positron length in pm. */
  std::optional<units::Length> positron;

  /*! \brief Optional Proton length in pm. */
  std::optional<units::Length> proton;
};

/*!
 * \brief Associates material-level overrides with an authoring material index.
 */
struct GGEMSMaterialProductionCuts {
  /*! \brief Index in the input material sequence. */
  std::uint32_t material_index;

  /*! \brief Per-channel material overrides. */
  GGEMSProductionCutLengths lengths;
};

/*! \brief Defines the initial 1 mm production-cut length for every channel. */
inline constexpr auto k_default_production_cut_length =
  units::operator""_mm(1ULL);

/*!
 * \brief Combines complete global defaults with optional material overrides.
 *
 * The default global values are all 1 mm. If materials contains repeated
 * indices, resolution uses the first matching entry. The current public setter
 * updates only global values; these records also support host-side resolution.
 */
struct GGEMSProductionCutPolicy {
  /*! \brief Global defaults, initially 1 mm for every channel. */
  GGEMSProductionCutLengths global{
    .gamma = k_default_production_cut_length,
    .electron = k_default_production_cut_length,
    .positron = k_default_production_cut_length,
    .proton = k_default_production_cut_length,
  };

  /*! \brief Ordered material overrides. */
  std::vector<GGEMSMaterialProductionCuts> materials;
};

/*!
 * \brief Supplies one authoring material index and optional volume-level cuts.
 */
struct GGEMSProductionCutContext {
  /*! \brief Index in the input material sequence. */
  std::uint32_t material_index;

  /*! \brief Per-channel context overrides. */
  GGEMSProductionCutLengths volume;
};

/*!
 * \brief Stores resolved lengths in Gamma, Electron, Positron, Proton order.
 */
using GGEMSResolvedProductionCutLengths = std::array<units::Length, 4U>;

/*!
 * \brief Carries effective lengths and their winning scopes in channel order.
 */
struct GGEMSResolvedProductionCuts {
  /*! \brief Effective lengths in canonical pm. */
  GGEMSResolvedProductionCutLengths lengths;

  /*! \brief Scope selected for each channel. */
  std::array<GGEMSProductionCutScope, 4U> scopes;
};

/*!
 * \brief Updates only the supplied channels of the process-wide global cut
 * policy.
 *
 * Unspecified channels retain their previous values. This operation does not
 * convert lengths or validate converter domains. Shared policy access is not
 * synchronized; configuration changes must not race with readers.
 *
 * \param[in] cuts Optional new global lengths in canonical pm.
 */
auto SetProductionCuts(GGEMSProductionCutLengths const &cuts) -> void;

/*!
 * \brief Returns a live read-only reference to the process-wide cut policy.
 *
 * The reference has process lifetime and observes subsequent
 * SetProductionCuts() updates; it is not an independent policy snapshot.
 *
 * \return The current global policy and its material-override collection.
 */
[[nodiscard]] auto GetProductionCutPolicy() noexcept
  -> GGEMSProductionCutPolicy const &;

/*!
 * \brief Resolves each channel with Volume, then Material, then Global
 * precedence.
 *
 * Uses the first matching material override. Resolving a length does not check
 * its converter domain or perform length-to-energy conversion.
 *
 * \pre Every channel must have a value at some applicable scope; missing final
 * values are dereferenced without a check.
 *
 * \param[in] policy Global defaults and material overrides.
 * \param[in] context Authoring material index and optional volume overrides.
 * \return Effective lengths and winning scopes in the common four-channel
 * order.
 */
[[nodiscard]] auto
ResolveProductionCuts(GGEMSProductionCutPolicy const &policy,
                      GGEMSProductionCutContext const &context)
  -> GGEMSResolvedProductionCuts;

} // namespace ggems::core::processes
