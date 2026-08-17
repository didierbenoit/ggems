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
 * \brief Declares the GGEMS random configuration and state initializer.
 *
 * Provides engine selection, seed configuration, OpenCL build metadata, state-range validation, and deterministic random-state initialization.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

/// \endcond
#include "GGEMS/random/GGEMSRandomEngine.hh"

namespace ggems::core::random {

/*!
 * \brief Configures the GGEMS random engine and initializes deterministic stream states.
 *
 * A GGEMSRandom object owns the selected engine and seed. It can initialize contiguous state storage for independent logical stream identifiers and supplies the build definition required by the generic OpenCL random API.
 */
class GGEMSRandom {
public:
  /*!
   * \brief Constructs the default Philox configuration with the default GGEMS seed.
   */
  GGEMSRandom();

  /*!
   * \brief Selects the random engine by identifier.
   *
   * \param[in] engine Random engine to use.
   * \return Reference to this configuration.
   */
  auto SetEngine(GGEMSRandomEngine engine) noexcept -> GGEMSRandom &;
  /*!
   * \brief Selects the random engine from a user-facing name.
   *
   * \param[in] engine_name Engine name or supported alias.
   * \return Reference to this configuration.
   *
   * \throws ggems::core::GGEMSRecoverable If the engine name is unsupported.
   */
  auto SetEngine(std::string_view engine_name) -> GGEMSRandom &;

  /*!
   * \brief Returns the selected random engine.
   *
   * \return Selected random-engine identifier.
   */
  [[nodiscard]] auto GetEngine() const noexcept -> GGEMSRandomEngine;
  /*!
   * \brief Returns the canonical name of the selected random engine.
   *
   * \return Canonical random-engine name.
   */
  [[nodiscard]] auto GetEngineName() const -> std::string;

  /*!
   * \brief Sets the deterministic seed used to initialize stream states.
   *
   * \param[in] seed New seed value.
   * \return Reference to this configuration.
   */
  auto SetSeed(std::uint64_t seed) noexcept -> GGEMSRandom &;
  /*!
   * \brief Returns the configured random seed.
   *
   * \return Current seed value.
   */
  [[nodiscard]] auto GetSeed() const noexcept -> std::uint64_t;

  /*!
   * \brief Returns the OpenCL numeric identifier of the selected engine.
   *
   * \return OpenCL engine identifier.
   */
  [[nodiscard]] auto GetKernelEngineId() const noexcept -> std::uint32_t;
  /*!
   * \brief Builds the OpenCL preprocessor definition selecting the random engine.
   *
   * \return Build option defining GGEMS_RANDOM_ENGINE.
   */
  [[nodiscard]] auto GetKernelBuildDefinition() const -> std::string;

  /*!
   * \brief Returns the state size of the selected random engine.
   *
   * \return State size in bytes, or zero for an unsupported engine value.
   */
  [[nodiscard]] auto GetStateSize() const noexcept -> std::size_t;

  /*!
   * \brief Validates a contiguous logical stream-identifier range.
   *
   * \param[in] first_stream_id First logical stream identifier.
   * \param[in] state_count Number of stream states in the range.
   *
   * \throws ggems::core::GGEMSRecoverable If the range overflows uint64_t or a JKISS stream identifier exceeds uint32_t.
   * \throws ggems::core::GGEMSInternal If the selected engine has no valid state size.
   */
  auto ValidateStateRange(std::uint64_t first_stream_id,
                          std::size_t state_count) const -> void;

  /*!
   * \brief Initializes contiguous engine states for logical stream identifiers.
   *
   * State \c i is initialized for stream \p first_stream_id + \c i using the current engine and seed.
   *
   * \param[in] first_stream_id Identifier assigned to the first state.
   * \param[out] state_storage Writable byte storage whose size must be an exact multiple of the selected engine state size.
   *
   * \throws ggems::core::GGEMSRecoverable If the storage size or stream range is invalid.
   * \throws ggems::core::GGEMSInternal If the selected engine cannot be initialized.
   */
  auto InitializeStates(std::uint64_t first_stream_id,
                        std::span<std::byte> state_storage) const -> void;

  /*!
   * \brief Builds human-readable summary lines for the random configuration.
   *
   * \return Summary containing engine, seed, state size, and OpenCL API metadata.
   */
  [[nodiscard]] auto BuildSummaryLines() const -> std::vector<std::string>;
  /*!
   * \brief Writes the random configuration summary to the GGEMS logger.
   */
  void Verbose() const;

private:
  /*!
   * \brief Default deterministic GGEMS random seed.
   */
  static constexpr std::uint64_t k_default_seed{77'777ULL};

  /*!
   * \brief Currently selected random engine.
   */
  GGEMSRandomEngine engine_{GGEMSRandomEngine::Philox};
  /*!
   * \brief Seed used to initialize deterministic stream states.
   */
  std::uint64_t seed_{k_default_seed};
};

} // namespace ggems::core::random
