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
 * \brief Unit tests for GGEMS banner rendering.
 *
 * Validates the complete banner output in both ASCII and Unicode encoding modes.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string_view>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/logging/GGEMSLogger.hh"
#include "GGEMS/render/GGEMSBanner.hh"
#include "GGEMS/render/GGEMSColorNames.hh"

#include "GGEMSScopedLoggerEncoding.hh"

/// \cond

namespace {

// =============================================================================
// =============================================================================

constexpr std::size_t number_lines = 17U;

// =============================================================================
// =============================================================================

using BannerSnapshot = std::array<std::u32string_view, number_lines>;
using ggems::test::ScopedLoggerEncoding;

// =============================================================================
// =============================================================================

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

auto ExpectBanner(ggems::core::Encoding encoding,
                  BannerSnapshot const &expected_lines) -> void {
  ScopedLoggerEncoding const scoped_encoding{encoding};
  auto const lines = ggems::render::BuildBannerLines();

  ASSERT_EQ(lines.size(), expected_lines.size());

  for (std::size_t line_index = 0U; line_index < lines.size(); ++line_index) {
    SCOPED_TRACE(line_index);
    ASSERT_EQ(lines[line_index].segments.size(), 1U);

    auto const &segment = lines[line_index].segments.front();
    EXPECT_EQ(segment.color, ggems::render::GREEN_Acid);
    EXPECT_EQ(segment.text, expected_lines[line_index]);
  }
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSBannerTest, BuildsCompleteAsciiBanner) {
  ExpectBanner(ggems::core::Encoding::Ascii, ascii_banner);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBannerTest, BuildsCompleteUnicodeBanner) {
  ExpectBanner(ggems::core::Encoding::Unicode, unicode_banner);
}
/// \endcond
