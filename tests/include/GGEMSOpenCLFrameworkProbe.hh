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
 * \brief Shared OpenCL framework probe metadata for tests.
 *
 * Provides the common framework-probe kernel name and the filesystem location used by OpenCL program, kernel, and profiler tests.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <string>
#include <filesystem>
/// \endcond

namespace ggems::test {

/*!
 * \brief Basename of the shared OpenCL framework probe kernel.
 */
inline const std::string k_opencl_framework_probe_name{
    "opencl_framework_probe"};

/*!
 * \brief Returns the directory containing shared OpenCL test kernels.
 *
 * \return Path to the test-kernel directory below GGEMS_TEST_KERNEL_ROOT.
 */
[[nodiscard]] inline auto GetOpenCLFrameworkProbeRoot()
    -> std::filesystem::path {
  return std::filesystem::path{GGEMS_TEST_KERNEL_ROOT} / "tests";
}
} // namespace ggems::test
