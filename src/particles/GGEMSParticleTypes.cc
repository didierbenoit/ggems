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
 * \brief Implements particle-species labels, symbols, and name parsing.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cctype>
#include <format>
#include <string>
#include <string_view>

#include "GGEMS/particles/GGEMSParticleTypes.hh"
#include "GGEMS/GGEMSException.hh"

namespace ggems::core::particles {
namespace {

// =============================================================================
// =============================================================================

/*!
 * \brief Lowercases a name after removing ASCII spaces and underscores.
 * \param[in] particle_name Borrowed particle name.
 * \return An owned normalized byte string; other whitespace is retained.
 */
auto NormalizeParticleName(std::string_view particle_name) -> std::string {
  std::string normalized;
  normalized.reserve(particle_name.size());

  for (char character : particle_name) {
    if (character == ' ' || character == '_') {
      continue;
    }

    normalized.push_back(
      static_cast<char>(std::tolower(static_cast<unsigned char>(character))));
  }

  return normalized;
}

} // namespace

// =============================================================================
// =============================================================================

auto ToLongName(GGEMSParticleType particle_type) -> std::string_view {
  switch (particle_type) {
  case GGEMSParticleType::Unknown:
    return "Unknown";
  case GGEMSParticleType::Aionino:
    return "Aionino";
  case GGEMSParticleType::Gamma:
    return "Gamma";
  case GGEMSParticleType::Electron:
    return "Electron";
  case GGEMSParticleType::Positron:
    return "Positron";
  case GGEMSParticleType::Proton:
    return "Proton";
  case GGEMSParticleType::Neutron:
    return "Neutron";
  case GGEMSParticleType::Alpha:
    return "Alpha";
  }

  return "Unknown";
}

// =============================================================================
// =============================================================================

auto ToShortName(GGEMSParticleType particle_type) -> std::string_view {
  switch (particle_type) {
  case GGEMSParticleType::Unknown:
    return "?";
  case GGEMSParticleType::Aionino:
    return "l";
  case GGEMSParticleType::Gamma:
    return "g";
  case GGEMSParticleType::Electron:
    return "b-";
  case GGEMSParticleType::Positron:
    return "b+";
  case GGEMSParticleType::Proton:
    return "p";
  case GGEMSParticleType::Neutron:
    return "n";
  case GGEMSParticleType::Alpha:
    return "a";
  }

  return "?";
}

// =============================================================================
// =============================================================================

auto ToAsciiSymbol(GGEMSParticleType particle_type) -> std::u32string_view {
  switch (particle_type) {
  case GGEMSParticleType::Unknown:
    return U"?";
  case GGEMSParticleType::Aionino:
    return U"l";
  case GGEMSParticleType::Gamma:
    return U"g";
  case GGEMSParticleType::Electron:
    return U"e-";
  case GGEMSParticleType::Positron:
    return U"e+";
  case GGEMSParticleType::Proton:
    return U"p";
  case GGEMSParticleType::Neutron:
    return U"n";
  case GGEMSParticleType::Alpha:
    return U"a";
  }

  return U"?";
}

// =============================================================================
// =============================================================================

auto ToUnicodeSymbol(GGEMSParticleType particle_type) -> std::u32string_view {
  switch (particle_type) {
  case GGEMSParticleType::Unknown:
    return U"?";
  case GGEMSParticleType::Aionino:
    return U"λ";
  case GGEMSParticleType::Gamma:
    return U"γ";
  case GGEMSParticleType::Electron:
    return U"β-";
  case GGEMSParticleType::Positron:
    return U"β+";
  case GGEMSParticleType::Proton:
    return U"p";
  case GGEMSParticleType::Neutron:
    return U"ν";
  case GGEMSParticleType::Alpha:
    return U"α";
  }

  return U"?";
}

// =============================================================================
// =============================================================================

auto ParseParticleType(std::string_view particle_name) -> GGEMSParticleType {
  std::string normalized = NormalizeParticleName(particle_name);

  if (normalized == "aionino" || normalized == "l") {
    return GGEMSParticleType::Aionino;
  }

  if (normalized == "gamma" || normalized == "photon" || normalized == "g") {
    return GGEMSParticleType::Gamma;
  }

  if (normalized == "electron" || normalized == "e-" || normalized == "b-") {
    return GGEMSParticleType::Electron;
  }

  if (normalized == "positron" || normalized == "e+" || normalized == "b+") {
    return GGEMSParticleType::Positron;
  }

  if (normalized == "proton" || normalized == "p") {
    return GGEMSParticleType::Proton;
  }

  if (normalized == "neutron" || normalized == "n") {
    return GGEMSParticleType::Neutron;
  }

  if (normalized == "alpha" || normalized == "a") {
    return GGEMSParticleType::Alpha;
  }

  throw ggems::core::GGEMSRecoverable(
    std::format("Unsupported GGEMS particle type '{}'.", particle_name));
}

} // namespace ggems::core::particles
