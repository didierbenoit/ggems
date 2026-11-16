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
 * \brief Shares canonical palette names between host authoring and Python.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <optional>
#include <span>
#include <string_view>

#include "GGEMS/render/GGEMSColorBlue.hh"
#include "GGEMS/render/GGEMSColorCyan.hh"
#include "GGEMS/render/GGEMSColorGray.hh"
#include "GGEMS/render/GGEMSColorGreen.hh"
#include "GGEMS/render/GGEMSColorMagenta.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColorOrange.hh"
#include "GGEMS/render/GGEMSColorRed.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/render/GGEMSColorWhite.hh"
#include "GGEMS/render/GGEMSColorYellow.hh"

namespace ggems::render {

// =============================================================================
// =============================================================================

auto GetNamedColors() -> std::span<NamedColor const> {
  using Catalog =
    std::array<NamedColor, color_family_count * color_shade_count>;
  static auto const colors = [] -> Catalog {
    Catalog entries;
    std::size_t index{0U};
    /// \cond NamedPaletteMacros
#define GGEMS_NAMED_COLOR(NAME, R, G, B)                                       \
  entries[index++] = NamedColor{.name = "gray_" #NAME, .key = GRAY_##NAME};
    GGEMS_COLOR_GRAY_SHADES(GGEMS_NAMED_COLOR)
#undef GGEMS_NAMED_COLOR

#define GGEMS_NAMED_COLOR(NAME, R, G, B)                                       \
  entries[index++] = NamedColor{.name = "red_" #NAME, .key = RED_##NAME};
    GGEMS_COLOR_RED_SHADES(GGEMS_NAMED_COLOR)
#undef GGEMS_NAMED_COLOR

#define GGEMS_NAMED_COLOR(NAME, R, G, B)                                       \
  entries[index++] = NamedColor{.name = "orange_" #NAME, .key = ORANGE_##NAME};
    GGEMS_COLOR_ORANGE_SHADES(GGEMS_NAMED_COLOR)
#undef GGEMS_NAMED_COLOR

#define GGEMS_NAMED_COLOR(NAME, R, G, B)                                       \
  entries[index++] = NamedColor{.name = "yellow_" #NAME, .key = YELLOW_##NAME};
    GGEMS_COLOR_YELLOW_SHADES(GGEMS_NAMED_COLOR)
#undef GGEMS_NAMED_COLOR

#define GGEMS_NAMED_COLOR(NAME, R, G, B)                                       \
  entries[index++] = NamedColor{.name = "green_" #NAME, .key = GREEN_##NAME};
    GGEMS_COLOR_GREEN_SHADES(GGEMS_NAMED_COLOR)
#undef GGEMS_NAMED_COLOR

#define GGEMS_NAMED_COLOR(NAME, R, G, B)                                       \
  entries[index++] = NamedColor{.name = "cyan_" #NAME, .key = CYAN_##NAME};
    GGEMS_COLOR_CYAN_SHADES(GGEMS_NAMED_COLOR)
#undef GGEMS_NAMED_COLOR

#define GGEMS_NAMED_COLOR(NAME, R, G, B)                                       \
  entries[index++] = NamedColor{.name = "blue_" #NAME, .key = BLUE_##NAME};
    GGEMS_COLOR_BLUE_SHADES(GGEMS_NAMED_COLOR)
#undef GGEMS_NAMED_COLOR

#define GGEMS_NAMED_COLOR(NAME, R, G, B)                                       \
  entries[index++] =                                                           \
    NamedColor{.name = "magenta_" #NAME, .key = MAGENTA_##NAME};
    GGEMS_COLOR_MAGENTA_SHADES(GGEMS_NAMED_COLOR)
#undef GGEMS_NAMED_COLOR

#define GGEMS_NAMED_COLOR(NAME, R, G, B)                                       \
  entries[index++] = NamedColor{.name = "white_" #NAME, .key = WHITE_##NAME};
    GGEMS_COLOR_WHITE_SHADES(GGEMS_NAMED_COLOR)
#undef GGEMS_NAMED_COLOR
    /// \endcond

    for (auto &entry : entries) {
      for (auto &character : entry.name) {
        if (character >= 'A' && character <= 'Z') {
          character = static_cast<char>(character - 'A' + 'a');
        }
      }
    }
    return entries;
  }();
  return colors;
}

// =============================================================================
// =============================================================================

auto ParseColorName(std::string_view name) -> std::optional<ColorKey> {
  for (auto const &entry : GetNamedColors()) {
    if (entry.name == name) {
      return entry.key;
    }
  }
  return std::nullopt;
}

// =============================================================================
// =============================================================================

auto GetColorName(ColorKey key) -> std::string_view {
  for (auto const &entry : GetNamedColors()) {
    if (entry.key == key) {
      return entry.name;
    }
  }
  return {};
}

} // namespace ggems::render
