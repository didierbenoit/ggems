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
 * \brief Unit tests for the OpenCL cache fingerprint helper.
 *
 * Validates the internal FNV-1a 64-bit implementation against canonical byte sequences, including empty input, embedded NUL bytes, and non-ASCII byte values.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <bit>
#include <cstdint>
#include <string_view>

#include <gtest/gtest.h>

/// \endcond
#include "GGEMS/opencl/GGEMSOpenCLCacheFingerprint.hh"

/// \cond

namespace {

struct FNV1a64Case {
  char const *label;
  std::string_view bytes;
  std::uint64_t expected_hash;
};

static_assert(noexcept(ggems::ocl::detail::HashFNV1a64(std::string_view{})));

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSOpenCLCacheFingerprintTest, MatchesCanonicalFNV1a64Vectors) {
  char const byte_ff = std::bit_cast<char>(std::uint8_t{0xffU});
  auto const byte_ff_view = std::string_view{&byte_ff, 1U};

  std::array<FNV1a64Case, 5U> const test_cases{{
      {.label = "empty input",
       .bytes = std::string_view{},
       .expected_hash = 0xcbf29ce484222325ULL},
      {.label = "a",
       .bytes = std::string_view{"a"},
       .expected_hash = 0xaf63dc4c8601ec8cULL},
      {.label = "hello",
       .bytes = std::string_view{"hello"},
       .expected_hash = 0xa430d84680aabd0bULL},
      {.label = "byte 0xff",
       .bytes = byte_ff_view,
       .expected_hash = 0xaf64724c8602eb6eULL},
      {.label = "embedded NUL",
       .bytes = std::string_view{"a\0b", 3U},
       .expected_hash = 0xe5d29919042666b2ULL},
  }};

  for (auto const &test_case : test_cases) {
    SCOPED_TRACE(test_case.label);
    EXPECT_EQ(ggems::ocl::detail::HashFNV1a64(test_case.bytes),
              test_case.expected_hash);
  }
}
/// \endcond
