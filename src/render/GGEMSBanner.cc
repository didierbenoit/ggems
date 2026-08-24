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
 * \brief Implements construction of the ASCII and Unicode GGEMS banner.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <string>
#include <string_view>
#include <utility>
#include <vector>
#include <cstddef>
/// \endcond

#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/render/GGEMSColor.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSVisualLine.hh"

namespace {

// =============================================================================
// =============================================================================

/*!
 * \brief Number of text rows in each banner snapshot.
 */
constexpr std::size_t number_lines = 17U;

/*!
 * \brief Fixed-size view of all text rows forming one banner variant.
 */
using BannerSnapshot = std::array<std::u32string_view, number_lines>;

// =============================================================================
// =============================================================================

/*!
 * \brief ASCII-only GGEMS banner used when ASCII output is selected.
 */
constexpr BannerSnapshot ascii_banner{{
    U"+****************************************************+",
    U"*                                                    *",
    U"*    ######   ######  ####### ###    ### #######     *",
    U"*   ##       ##       ##      ####  #### ##          *",
    U"*   ##   ### ##   ### #####   ## #### ## #######     *",
    U"*   ##    ## ##    ## ##      ##  ##  ##      ##     *",
    U"*    ######   ######  ####### ##      ## #######     *",
    U"*   ******** ******** ******* ********** *******     *",
    U"*                                                    *",
    U"+----------------------------------------------------+",
    U"*                                                    *",
    U"*      GPU Geant4-based Monte Carlo Simulations      *",
    U"*    Version 2.0 . GGEMS Team . https://ggems.fr     *",
    U"*       Authors: Julien Bert & Didier Benoit         *",
    U"*   Copyright (C) 2026 Licensed under GNU GPL v3.0   *",
    U"*                                                    *",
    U"+****************************************************+",
}};

// =============================================================================
// =============================================================================

/*!
 * \brief Unicode GGEMS banner used when Unicode output is selected.
 */
constexpr BannerSnapshot unicode_banner{{
    U"╔════════════════════════════════════════════════════╗",
    U"║                                                    ║",
    U"║    ██████╗  ██████╗ ███████╗███╗   ███╗███████╗    ║",
    U"║   ██╔════╝ ██╔════╝ ██╔════╝████╗ ████║██╔════╝    ║",
    U"║   ██║  ███╗██║  ███╗█████╗  ██╔████╔██║███████╗    ║",
    U"║   ██║   ██║██║   ██║██╔══╝  ██║╚██╔╝██║╚════██║    ║",
    U"║   ╚██████╔╝╚██████╔╝███████╗██║ ╚═╝ ██║███████║    ║",
    U"║    ╚═════╝  ╚═════╝ ╚══════╝╚═╝     ╚═╝╚══════╝    ║",
    U"║                                                    ║",
    U"╟────────────────────────────────────────────────────╢",
    U"║                                                    ║",
    U"║      GPU Geant4-based Monte Carlo Simulations      ║",
    U"║    Version 2.0 • GGEMS Team • https://ggems.fr     ║",
    U"║       Authors: Julien Bert & Didier Benoit         ║",
    U"║   Copyright (C) 2026 Licensed under GNU GPL v3.0   ║",
    U"║                                                    ║",
    U"╚════════════════════════════════════════════════════╝",
}};

// =============================================================================
// =============================================================================

/*!
 * \brief Color applied to every segment of the GGEMS banner.
 */
constexpr ggems::render::ColorKey banner_color = ggems::render::GREEN_Neon;

// =============================================================================
// =============================================================================

/*!
 * \brief Selects the banner snapshot for the active logger encoding.
 *
 * \return Reference to the ASCII or Unicode banner snapshot.
 */
[[nodiscard]] auto GetBannerSnapshot() noexcept -> BannerSnapshot const & {
  if (ggems::core::GGEMSLogger::GetInstance().GetEncoding() ==
      ggems::core::Encoding::Ascii) {
    return ascii_banner;
  }

  return unicode_banner;
}

} // namespace

namespace ggems::render {

// =============================================================================
// =============================================================================

auto BuildBannerLines() -> std::vector<WrappedLine> {
  auto const &snapshot = GetBannerSnapshot();

  std::vector<WrappedLine> lines;
  lines.reserve(snapshot.size());

  for (auto const text : snapshot) {
    WrappedLine line;
    line.segments.push_back(
        {.text = std::u32string{text}, .color = banner_color});
    lines.push_back(std::move(line));
  }

  return lines;
}

} // namespace ggems::render
