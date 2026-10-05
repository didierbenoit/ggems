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
 * \brief Tests canonical built-in registration, lookup and descriptions.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <array>
#include <cstddef>
#include <string>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/radioactivity/builtins/GGEMSBuiltInRadionuclides.hh"

namespace {

struct ExpectedBuiltIn {
  std::string_view name;
  long double half_life_seconds;
  std::size_t emission_count;
};

} // namespace

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInRadionuclides, DispatchesOnlyExactCanonicalNames) {
  constexpr std::array<ExpectedBuiltIn, 52U> expected{
    {
      {
        .name = "H-3",
        .half_life_seconds = 388'500'000.0L,
        .emission_count = 1U,
      },
      {
        .name = "C-14",
        .half_life_seconds = 179'900'000'000.0L,
        .emission_count = 1U,
      },
      {.name = "F-18", .half_life_seconds = 6584.04L, .emission_count = 3U},
      {.name = "C-11", .half_life_seconds = 1221.66L, .emission_count = 1U},
      {.name = "O-15", .half_life_seconds = 122.266L, .emission_count = 1U},
      {.name = "Ga-68", .half_life_seconds = 4'069.8L, .emission_count = 7U},
      {
        .name = "Co-60",
        .half_life_seconds = 166'340'000.0L,
        .emission_count = 7U,
      },
      {
        .name = "Lu-177",
        .half_life_seconds = 574'067.52L,
        .emission_count = 8U,
      },
      {.name = "I-123", .half_life_seconds = 47'604.24L, .emission_count = 4U},
      {.name = "I-124", .half_life_seconds = 360'806.4L, .emission_count = 13U},
      {
        .name = "I-125",
        .half_life_seconds = 5'131'123.2L,
        .emission_count = 4U,
      },
      {
        .name = "I-131",
        .half_life_seconds = 693'213.12L,
        .emission_count = 10U,
      },
      {
        .name = "Am-241",
        .half_life_seconds = 13'652'000'000.0L,
        .emission_count = 6U,
      },
      {.name = "Tc-99m", .half_life_seconds = 21'624.12L, .emission_count = 6U},
      {.name = "P-32", .half_life_seconds = 1'233'187.2L, .emission_count = 1U},
      {.name = "P-33", .half_life_seconds = 2'192'832.0L, .emission_count = 1U},
      {
        .name = "Co-57",
        .half_life_seconds = 23484384.00L,
        .emission_count = 12U,
      },
      {
        .name = "Ga-67",
        .half_life_seconds = 281776.3200L,
        .emission_count = 12U,
      },
      {.name = "Cu-67", .half_life_seconds = 222552.00L, .emission_count = 12U},
      {.name = "Sc-44", .half_life_seconds = 14292.00L, .emission_count = 5U},
      {
        .name = "Sc-47",
        .half_life_seconds = 289310.4000L,
        .emission_count = 5U,
      },
      {
        .name = "Mn-52",
        .half_life_seconds = 483148.800L,
        .emission_count = 26U,
      },
      {.name = "Br-76", .half_life_seconds = 57960.0L, .emission_count = 96U},
      {.name = "Na-24", .half_life_seconds = 53848.800L, .emission_count = 12U},
      {.name = "S-35", .half_life_seconds = 7538400.00L, .emission_count = 1U},
      {
        .name = "Ca-45",
        .half_life_seconds = 14052096.00L,
        .emission_count = 3U,
      },
      {
        .name = "Hg-203",
        .half_life_seconds = 4025721.600L,
        .emission_count = 5U,
      },
      {
        .name = "Tl-201",
        .half_life_seconds = 262837.4400L,
        .emission_count = 8U,
      },
      {
        .name = "Xe-133",
        .half_life_seconds = 453375.3600L,
        .emission_count = 11U,
      },
      {.name = "N-13", .half_life_seconds = 597.5340L, .emission_count = 1U},
      {.name = "Rb-82", .half_life_seconds = 75.9120L, .emission_count = 17U},
      {
        .name = "In-111",
        .half_life_seconds = 242343.3600L,
        .emission_count = 5U,
      },
      {
        .name = "Cu-64",
        .half_life_seconds = 45721.4400L,
        .emission_count = 5U,
      },
      {
        .name = "Zr-89",
        .half_life_seconds = 282312.00L,
        .emission_count = 3U,
      },
      {
        .name = "Ra-223",
        .half_life_seconds = 987552.00L,
        .emission_count = 30U,
      },
      {
        .name = "Sr-89",
        .half_life_seconds = 4369248.00L,
        .emission_count = 3U,
      },
      {
        .name = "Sm-153",
        .half_life_seconds = 166626.72000L,
        .emission_count = 35U,
      },
      {
        .name = "Re-186",
        .half_life_seconds = 321287.0400L,
        .emission_count = 15U,
      },
      {
        .name = "Re-188",
        .half_life_seconds = 61218.000L,
        .emission_count = 41U,
      },
      {
        .name = "Ho-166",
        .half_life_seconds = 96508.800L,
        .emission_count = 26U,
      },
      {
        .name = "Ac-225",
        .half_life_seconds = 856846.0800L,
        .emission_count = 94U,
      },
      {
        .name = "At-211",
        .half_life_seconds = 25977.600L,
        .emission_count = 9U,
      },
      {
        .name = "Pb-212",
        .half_life_seconds = 38268.000L,
        .emission_count = 10U,
      },
      {
        .name = "Bi-212",
        .half_life_seconds = 3632.40L,
        .emission_count = 31U,
      },
      {
        .name = "Bi-213",
        .half_life_seconds = 2735.40L,
        .emission_count = 17U,
      },
      {
        .name = "Pd-103",
        .half_life_seconds = 1468800.00L,
        .emission_count = 9U,
      },
      {
        .name = "Cs-131",
        .half_life_seconds = 836438.400L,
        .emission_count = 1U,
      },
      {
        .name = "Ir-192",
        .half_life_seconds = 6378652.800L,
        .emission_count = 35U,
      },
      {
        .name = "Cs-137",
        .half_life_seconds = 947275801.882329600L,
        .emission_count = 3U,
      },
      {
        .name = "Ru-106",
        .half_life_seconds = 32097600.0L,
        .emission_count = 1U,
      },
      {
        .name = "Mo-99",
        .half_life_seconds = 237418.5600L,
        .emission_count = 25U,
      },
      {
        .name = "Sr-82",
        .half_life_seconds = 2189980.800L,
        .emission_count = 1U,
      },
    },
  };

  for (auto const &entry : expected) {
    auto definition =
      ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide(
        entry.name);
    ASSERT_TRUE(definition.has_value());
    EXPECT_EQ(definition->GetCanonicalName(), entry.name);
    EXPECT_EQ(definition->GetHalfLifeSeconds(), entry.half_life_seconds);
    EXPECT_EQ(definition->GetEmissions().size(), entry.emission_count);
  }

  EXPECT_FALSE(
    ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide("F18")
      .has_value());
  EXPECT_FALSE(
    ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide("f-18")
      .has_value());
  EXPECT_FALSE(
    ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide(" F-18 ")
      .has_value());
  EXPECT_FALSE(
    ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide("P32")
      .has_value());
  EXPECT_FALSE(
    ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide("p-32")
      .has_value());
  EXPECT_FALSE(
    ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide(" P-32 ")
      .has_value());
  EXPECT_FALSE(
    ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide("P33")
      .has_value());
  EXPECT_FALSE(
    ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide("p-33")
      .has_value());
  EXPECT_FALSE(
    ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide(" P-33 ")
      .has_value());
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInRadionuclides,
     ListsEveryAvailableCanonicalNameInDispatchOrder) {
  constexpr std::array<std::string_view, 52U> expected{
    "H-3",    "C-14",   "F-18",   "C-11",   "O-15",   "Ga-68",  "Co-60",
    "Lu-177", "I-123",  "I-124",  "I-125",  "I-131",  "Am-241", "Tc-99m",
    "P-32",   "P-33",   "Co-57",  "Ga-67",  "Cu-67",  "Sc-44",  "Sc-47",
    "Mn-52",  "Br-76",  "Na-24",  "S-35",   "Ca-45",  "Hg-203", "Tl-201",
    "Xe-133", "N-13",   "Rb-82",  "In-111", "Cu-64",  "Zr-89",  "Ra-223",
    "Sr-89",  "Sm-153", "Re-186", "Re-188", "Ho-166", "Ac-225", "At-211",
    "Pb-212", "Bi-212", "Bi-213", "Pd-103", "Cs-131", "Ir-192", "Cs-137",
    "Ru-106", "Mo-99",  "Sr-82",
  };

  auto const available =
    ggems::core::radioactivity::builtins::GetAvailableRadionuclideNames();
  ASSERT_EQ(available.size(), expected.size());

  for (std::size_t index = 0U; index < expected.size(); ++index) {
    EXPECT_EQ(available[index], expected[index]);
    EXPECT_TRUE(ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide(
                  available[index])
                  .has_value());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInRadionuclides, RejectsNoncanonicalAddedNames) {
  for (std::string_view const name : {
         "Co57",  "co-57",  " Co-57 ",  "Ga67",  "ga-67",  " Ga-67 ",
         "Cu67",  "cu-67",  " Cu-67 ",  "Sc44",  "sc-44",  " Sc-44 ",
         "Sc47",  "sc-47",  " Sc-47 ",  "Mn52",  "mn-52",  " Mn-52 ",
         "Br76",  "br-76",  " Br-76 ",  "Na24",  "na-24",  " Na-24 ",
         "S35",   "s-35",   " S-35 ",   "Ca45",  "ca-45",  " Ca-45 ",
         "Hg203", "hg-203", " Hg-203 ", "Tl201", "tl-201", " Tl-201 ",
         "Xe133", "xe-133", " Xe-133 ", "N13",   "n-13",   " N-13 ",
         "Rb82",  "rb-82",  " Rb-82 ",  "In111", "in-111", " In-111 ",
         "Cu64",  "cu-64",  " Cu-64 ",  "Zr89",  "zr-89",  " Zr-89 ",
         "Ra223", "ra-223", " Ra-223 ", "Sr89",  "sr-89",  " Sr-89 ",
         "Sm153", "sm-153", " Sm-153 ", "Re186", "re-186", " Re-186 ",
         "Re188", "re-188", " Re-188 ", "Ho166", "ho-166", " Ho-166 ",
         "Ac225", "ac-225", " Ac-225 ", "At211", "at-211", " At-211 ",
         "Pb212", "pb-212", " Pb-212 ", "Bi212", "bi-212", " Bi-212 ",
         "Bi213", "bi-213", " Bi-213 ", "Pd103", "pd-103", " Pd-103 ",
         "Cs131", "cs-131", " Cs-131 ", "Ir192", "ir-192", " Ir-192 ",
         "Cs137", "cs-137", " Cs-137 ", "Ru106", "ru-106", " Ru-106 ",
         "Mo99",  "mo-99",  " Mo-99 ",  "Sr82",  "sr-82",  " Sr-82 ",
       }) {
    EXPECT_FALSE(
      ggems::core::radioactivity::builtins::BuildBuiltInRadionuclide(name)
        .has_value());
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInRadionuclides, DescriptionRejectsUnknownCanonicalName) {
  EXPECT_THROW(
    static_cast<void>(
      ggems::core::radioactivity::builtins::DescribeBuiltInRadionuclide("F18")),
    ggems::core::GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBuiltInRadionuclides, DescriptionReflectsBuiltDefinition) {
  std::string const description =
    ggems::core::radioactivity::builtins::DescribeBuiltInRadionuclide("H-3");

  EXPECT_NE(description.find("H-3"), std::string::npos);
  EXPECT_NE(description.find("Electron | yield 1"), std::string::npos);
  EXPECT_NE(description.find("Regular spectrum | 38 bins"), std::string::npos);
  EXPECT_NE(description.find("18.5910000 keV"), std::string::npos);
  EXPECT_NE(description.find("489.2360000 eV"), std::string::npos);
  EXPECT_NE(description.find("Total yield    : 1 particles/decay"),
            std::string::npos);
}
