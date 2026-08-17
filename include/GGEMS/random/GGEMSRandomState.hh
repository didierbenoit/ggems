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
 * \brief Declares host-side random-engine state layouts.
 *
 * Defines ABI-stable host representations of the JKISS, PCG32, and Philox states mirrored by the OpenCL kernels.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstddef>
#include <cstdint>
#include <type_traits>
/// \endcond

namespace ggems::core::random {

/*!
 * \brief Stores the host/OpenCL ABI state of one JKISS stream.
 *
 * The layout is mirrored by GGEMSJKissState in the OpenCL random-state header.
 */
struct GGEMSJKissState {
  /*!
   * \brief JKISS congruential component.
   */
  std::uint32_t x{0U};
  /*!
   * \brief JKISS xorshift component.
   */
  std::uint32_t y{0U};
  /*!
   * \brief JKISS multiply-with-carry state component.
   */
  std::uint32_t z{0U};
  /*!
   * \brief JKISS multiply-with-carry state component.
   */
  std::uint32_t w{0U};
  /*!
   * \brief JKISS carry component.
   */
  std::uint32_t c{0U};
};

static_assert(std::is_standard_layout_v<GGEMSJKissState>);
static_assert(std::is_trivially_copyable_v<GGEMSJKissState>);
static_assert(alignof(GGEMSJKissState) == 4U);
static_assert(sizeof(GGEMSJKissState) == 5U * sizeof(std::uint32_t));
static_assert(offsetof(GGEMSJKissState, x) == 0U);
static_assert(offsetof(GGEMSJKissState, y) == 4U);
static_assert(offsetof(GGEMSJKissState, z) == 8U);
static_assert(offsetof(GGEMSJKissState, w) == 12U);
static_assert(offsetof(GGEMSJKissState, c) == 16U);

/*!
 * \brief Stores the host/OpenCL ABI state of one PCG32 stream.
 *
 * The layout is mirrored by GGEMSPCG32State in the OpenCL random-state header.
 */
struct GGEMSPCG32State {
  /*!
   * \brief Current PCG32 linear-congruential state.
   */
  std::uint64_t state{0U};
  /*!
   * \brief Odd PCG32 stream increment selecting the independent sequence.
   */
  std::uint64_t increment{0U};
};

static_assert(std::is_standard_layout_v<GGEMSPCG32State>);
static_assert(std::is_trivially_copyable_v<GGEMSPCG32State>);
static_assert(alignof(GGEMSPCG32State) == 8U);
static_assert(sizeof(GGEMSPCG32State) == 2U * sizeof(std::uint64_t));
static_assert(offsetof(GGEMSPCG32State, state) == 0U);
static_assert(offsetof(GGEMSPCG32State, increment) == 8U);

/*!
 * \brief Stores the host/OpenCL ABI state of one Philox stream.
 *
 * The counter identifies the current block and stream while the key is derived from the configured GGEMS seed.
 */
struct GGEMSPhiloxState {
  /*!
   * \brief Least-significant Philox counter word advanced for each generated block.
   */
  std::uint32_t counter_0{0U};
  /*!
   * \brief Second Philox counter word used for carry propagation.
   */
  std::uint32_t counter_1{0U};
  /*!
   * \brief Low 32 bits of the Philox stream identifier.
   */
  std::uint32_t counter_2{0U};
  /*!
   * \brief High 32 bits of the Philox stream identifier.
   */
  std::uint32_t counter_3{0U};
  /*!
   * \brief Low 32 bits of the Philox key.
   */
  std::uint32_t key_0{0};
  /*!
   * \brief High 32 bits of the Philox key.
   */
  std::uint32_t key_1{1};
};

static_assert(std::is_standard_layout_v<GGEMSPhiloxState>);
static_assert(std::is_trivially_copyable_v<GGEMSPhiloxState>);
static_assert(alignof(GGEMSPhiloxState) == 4U);
static_assert(sizeof(GGEMSPhiloxState) == 6U * sizeof(std::uint32_t));
static_assert(offsetof(GGEMSPhiloxState, counter_0) == 0U);
static_assert(offsetof(GGEMSPhiloxState, counter_1) == 4U);
static_assert(offsetof(GGEMSPhiloxState, counter_2) == 8U);
static_assert(offsetof(GGEMSPhiloxState, counter_3) == 12U);
static_assert(offsetof(GGEMSPhiloxState, key_0) == 16U);
static_assert(offsetof(GGEMSPhiloxState, key_1) == 20U);

} // namespace ggems::core::random
