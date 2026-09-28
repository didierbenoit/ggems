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
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstdint>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"

namespace {

[[noreturn]] auto ThrowRecoverableFromKnownLine(std::int32_t &expected_line)
  -> void {
  expected_line = static_cast<std::int32_t>(__LINE__ + 1);
  throw ggems::core::GGEMSRecoverable{"source-location marker"};
}

auto ExpectDiagnosticContains(std::string_view diagnostic,
                              std::string_view expected) -> void {
  EXPECT_NE(diagnostic.find(expected), std::string_view::npos);
}

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSExceptionTest, ReportsCategoryAndMessageInDiagnostic) {
  {
    ggems::core::GGEMSRecoverable const exception{"recoverable marker"};
    auto const diagnostic = std::string_view{exception.what()};

    ExpectDiagnosticContains(diagnostic, "Recoverable");
    ExpectDiagnosticContains(diagnostic, "recoverable marker");
  }

  {
    ggems::core::GGEMSInternal const exception{"internal marker"};
    auto const diagnostic = std::string_view{exception.what()};

    ExpectDiagnosticContains(diagnostic, "Internal");
    ExpectDiagnosticContains(diagnostic, "internal marker");
  }

  {
    ggems::core::GGEMSFatal const exception{"fatal marker"};
    auto const diagnostic = std::string_view{exception.what()};

    ExpectDiagnosticContains(diagnostic, "Fatal");
    ExpectDiagnosticContains(diagnostic, "fatal marker");
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSExceptionTest, CapturesDirectThrowSiteInDiagnostic) {
  std::int32_t expected_line = 0;

  try {
    ThrowRecoverableFromKnownLine(expected_line);
    FAIL() << "Expected GGEMSRecoverable.";
  } catch (ggems::core::GGEMSRecoverable const &exception) {
    auto const diagnostic = std::string_view{exception.what()};

    ExpectDiagnosticContains(diagnostic, "source-location marker");
    ExpectDiagnosticContains(diagnostic, "GGEMSExceptionTest.cc");
    ExpectDiagnosticContains(diagnostic, "ThrowRecoverableFromKnownLine");
    ExpectDiagnosticContains(
      diagnostic, "  Line     : " + std::to_string(expected_line) + "\n");
  }
}
