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
 * \brief Defines particle species, status codes, and display conversions.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <string_view>

/*!
 * \namespace ggems::core::particles
 * \brief Provides host particle records and shared numeric identifiers.
 */
namespace ggems::core::particles {
/*! \brief Reserved all-ones sentinel for an absent 32-bit identifier. */
constexpr std::uint32_t k_invalid_id_u32{0xFFFFFFFFU};

/*! \brief Reserved all-ones sentinel for an absent 64-bit identifier. */
constexpr std::uint64_t k_invalid_id_u64{0xFFFFFFFFFFFFFFFFULL};

/*!
 * \brief Identifies a particle species using codes shared with OpenCL.
 *
 * An enumerator identifies a species; it does not imply that its physical
 * transport processes are implemented.
 */
enum class GGEMSParticleType : std::uint8_t {
  /*! \brief Unrecognized or unspecified particle species. */
  Unknown = 0U,

  /*! \brief Diagnostic particle used by synthetic transport. */
  Aionino = 1U,

  /*! \brief Photon species. */
  Gamma = 2U,

  /*! \brief Electron species. */
  Electron = 3U,

  /*! \brief Positron species. */
  Positron = 4U,

  /*! \brief Proton species. */
  Proton = 5U,

  /*! \brief Neutron species. */
  Neutron = 6U,

  /*! \brief Alpha-particle species. */
  Alpha = 7U,
};

/*!
 * \brief Defines particle lifecycle status codes shared with OpenCL.
 *
 * These labels describe stored state, not the availability of a corresponding
 * physical process.
 */
enum class GGEMSParticleStatus : std::uint8_t {
  /*! \brief Inactive particle slot. */
  Inactive = 0U,

  /*! \brief Particle eligible for further transport. */
  Alive = 1U,

  /*! \brief Particle marked as terminated. */
  Killed = 2U,

  /*! \brief Particle marked as having left the world. */
  EscapedWorld = 3U,

  /*! \brief Particle marked as absorbed. */
  Absorbed = 4U,
};

/*!
 * \brief Widens a particle-species code to the kernel representation.
 * \param[in] particle_type Host species code; no validation is performed.
 * \return The underlying numeric code as uint32_t.
 */
constexpr auto ToKernelParticleType(GGEMSParticleType particle_type) noexcept
  -> std::uint32_t {
  return static_cast<std::uint32_t>(particle_type);
}

/*!
 * \brief Decodes a kernel particle-species code.
 * \param[in] particle_type Kernel species code.
 * \return The species for codes 1 through 7, or GGEMSParticleType::Unknown for
 * every other value.
 */
constexpr auto FromKernelParticleType(std::uint32_t particle_type) noexcept
  -> GGEMSParticleType {
  switch (particle_type) {
  case 1U:
    return GGEMSParticleType::Aionino;
  case 2U:
    return GGEMSParticleType::Gamma;
  case 3U:
    return GGEMSParticleType::Electron;
  case 4U:
    return GGEMSParticleType::Positron;
  case 5U:
    return GGEMSParticleType::Proton;
  case 6U:
    return GGEMSParticleType::Neutron;
  case 7U:
    return GGEMSParticleType::Alpha;
  default:
    return GGEMSParticleType::Unknown;
  }
}

/*!
 * \brief Widens a particle-status code to the kernel representation.
 * \param[in] status Host status code; no validation is performed.
 * \return The underlying numeric code as uint32_t.
 */
constexpr auto ToKernelParticleStatus(GGEMSParticleStatus status) noexcept
  -> std::uint32_t {
  return static_cast<std::uint32_t>(status);
}

/*!
 * \brief Returns the English particle-species name.
 * \param[in] particle_type Species to display.
 * \return A static-lifetime name, or "Unknown" for an unknown or invalid code.
 */
auto ToLongName(GGEMSParticleType particle_type) -> std::string_view;

/*!
 * \brief Returns a compact ASCII particle-species label.
 * \param[in] particle_type Species to display.
 * \return A static-lifetime label; electrons and positrons use "b-" and "b+".
 * Unknown or invalid codes return "?".
 */
auto ToShortName(GGEMSParticleType particle_type) -> std::string_view;

/*!
 * \brief Returns ASCII display characters in a UTF-32 view.
 * \param[in] particle_type Species to display.
 * \return A static-lifetime symbol; electrons and positrons use "e-" and "e+".
 * Unknown or invalid codes return U"?".
 */
[[nodiscard]] auto ToAsciiSymbol(GGEMSParticleType particle_type)
  -> std::u32string_view;

/*!
 * \brief Returns the Unicode particle display symbol.
 * \param[in] particle_type Species to display.
 * \return A static-lifetime UTF-32 view, or U"?" for an unknown or invalid
 * code.
 */
[[nodiscard]] auto ToUnicodeSymbol(GGEMSParticleType particle_type)
  -> std::u32string_view;

/*!
 * \brief Parses a supported particle name or ASCII alias.
 *
 * Removes ASCII spaces and underscores, then lowercases bytes. Accepted names
 * are Aionino, Gamma, Photon, Electron, Positron, Proton, Neutron, and Alpha;
 * aliases are l, g, e-/b-, e+/b+, p, n, and a, respectively. The display
 * fallback "Unknown" is not an accepted species.
 *
 * \param[in] particle_name Name or alias to parse.
 * \return The recognized particle species.
 * \throws ggems::core::GGEMSRecoverable If the normalized name is unsupported.
 */
auto ParseParticleType(std::string_view particle_name) -> GGEMSParticleType;

} // namespace ggems::core::particles
