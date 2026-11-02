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
 * \brief Parses test launch options without initializing OpenCL.
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMSOpenCLDeviceInventory.hh"

auto main(int argc, char **argv) -> int {
  try {
    std::string_view selector{"gpu:0"};
    if (auto const *environment = std::getenv("GGEMS_TEST_DEVICE")) {
      selector = environment;
    }

    for (int argument = 1; argument < argc; ++argument) {
      std::string_view const value{argv[argument]};
      if (value.starts_with("--ggems-device=")) {
        selector = value.substr(std::string_view{"--ggems-device="}.size());
      } else if (value == "--ggems-device") {
        if (++argument == argc) {
          throw std::invalid_argument{"--ggems-device requires a selector."};
        }
        selector = argv[argument];
      } else if (value.starts_with("--ggems-")) {
        throw std::invalid_argument{"Unknown GGEMS test launcher option."};
      }
    }

    ggems::test::opencl_device_request =
      ggems::test::ParseOpenCLDeviceRequest(selector);
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
  } catch (std::exception const &exception) {
    std::cerr << "GGEMS test launcher: " << exception.what() << '\n';
    return EXIT_FAILURE;
  }
}
