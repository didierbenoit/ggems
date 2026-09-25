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
 * \brief Defines OpenCL shared virtual memory kinds and related utilities.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <string_view>
#include <cstdint>
/// \endcond

/*!
 * \namespace ggems::ocl
 * \brief Provides the OpenCL backend components of GGEMS.
 */
namespace ggems::ocl {

/*! \brief Specifies the shared virtual memory mode used by an OpenCL buffer. */
enum class SVMMemoryKind : std::uint8_t {
  /*! \brief No SVM memory mode is selected. */
  None,

  /*! \brief Selects the SVM memory mode automatically. */
  Auto,

  /*! \brief Uses coarse-grain buffer SVM. */
  CoarseGrainBuffer,

  /*! \brief Uses fine-grain buffer SVM. */
  FineGrainBuffer,

  /*! \brief Uses fine-grain buffer SVM with atomic access. */
  FineGrainBufferAtomics,

  /*! \brief Uses fine-grain system SVM. */
  FineGrainSystem,
};

/*!
 * \brief Returns the name of an SVM memory kind.
 *
 * \param[in] kind SVM memory kind to convert.
 * \return A static-lifetime name, or "Unknown" for an invalid enumerator.
 */
[[nodiscard]] constexpr auto ToString(SVMMemoryKind kind) noexcept
  -> std::string_view {
  switch (kind) {
  case SVMMemoryKind::None:
    return "None";
  case SVMMemoryKind::Auto:
    return "Auto";
  case SVMMemoryKind::CoarseGrainBuffer:
    return "CoarseGrainBuffer";
  case SVMMemoryKind::FineGrainBuffer:
    return "FineGrainBuffer";
  case SVMMemoryKind::FineGrainBufferAtomics:
    return "FineGrainBufferAtomics";
  case SVMMemoryKind::FineGrainSystem:
    return "FineGrainSystem";
  }

  return "Unknown";
}

/*!
 * \brief Checks whether an SVM memory kind requires explicit host mapping.
 *
 * \param[in] kind SVM memory kind to check.
 * \return True only for CoarseGrainBuffer. This predicate does not validate
 * support or synchronize memory.
 */
[[nodiscard]] constexpr auto RequiresExplicitMap(SVMMemoryKind kind) noexcept
  -> bool {
  return kind == SVMMemoryKind::CoarseGrainBuffer;
}

} // namespace ggems::ocl
