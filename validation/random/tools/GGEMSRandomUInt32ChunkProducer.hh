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

#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

#include "GGEMS/random/GGEMSRandomEngine.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"

namespace ggems::ocl {
class GGEMSOpenCLContext;
}

namespace ggems::validation::random {

enum class RandomUInt32StreamLayout : std::uint8_t {
  WorkerMajor = 0U,
  Interleaved = 1U,
};

[[nodiscard]] auto ToString(RandomUInt32StreamLayout layout)
  -> std::string_view;

[[nodiscard]] auto ParseRandomUInt32StreamLayout(std::string_view value)
  -> RandomUInt32StreamLayout;

struct RandomUInt32StreamSpecification {
  ggems::core::random::GGEMSRandomEngine engine{
    ggems::core::random::GGEMSRandomEngine::Philox};
  std::uint64_t seed{77'777ULL};
  std::uint64_t stream_offset{0ULL};
  std::uint32_t worker_count{1U};
  std::uint32_t samples_per_worker{1U};
  RandomUInt32StreamLayout layout{RandomUInt32StreamLayout::Interleaved};
  std::size_t local_size{64U};
  ggems::units::Bytes maximum_value_buffer_size{256ULL * 1024ULL * 1024ULL};
};

enum class RandomUInt32ChunkStrategy : std::uint8_t {
  SampleDepth = 0U,
  WorkerGroups,
};

[[nodiscard]] auto ToString(RandomUInt32ChunkStrategy strategy)
  -> std::string_view;

struct RandomUInt32ChunkPlan {
  RandomUInt32ChunkStrategy strategy{RandomUInt32ChunkStrategy::SampleDepth};
  ggems::units::Bytes requested_max_value_buffer_size{0ULL};
  ggems::units::Bytes effective_max_value_buffer_size{0ULL};
  ggems::units::Bytes device_max_allocation_size{0ULL};
  ggems::units::Bytes state_buffer_size{0ULL};
  ggems::units::Bytes value_buffer_size{0ULL};
  std::uint32_t workers_per_chunk{0U};
  std::uint32_t samples_per_worker_per_chunk{0U};
  std::uint64_t chunk_count{0ULL};
};

class GGEMSRandomUInt32ChunkProducer {
public:
  GGEMSRandomUInt32ChunkProducer(
    RandomUInt32StreamSpecification specification,
    ggems::ocl::GGEMSOpenCLContext &context,
    std::optional<std::uint64_t> output_word_limit = std::nullopt);

  ~GGEMSRandomUInt32ChunkProducer();

  GGEMSRandomUInt32ChunkProducer(GGEMSRandomUInt32ChunkProducer const &) =
    delete;

  GGEMSRandomUInt32ChunkProducer(GGEMSRandomUInt32ChunkProducer &&) noexcept =
    delete;

  auto operator=(GGEMSRandomUInt32ChunkProducer const &)
    -> GGEMSRandomUInt32ChunkProducer & = delete;

  auto operator=(GGEMSRandomUInt32ChunkProducer &&) noexcept
    -> GGEMSRandomUInt32ChunkProducer & = delete;

  [[nodiscard]] auto NextChunk() -> std::span<std::uint32_t const>;

  [[nodiscard]] auto GetChunkPlan() const noexcept
    -> RandomUInt32ChunkPlan const &;

  [[nodiscard]] auto GetReturnedWordCount() const noexcept -> std::uint64_t;

  [[nodiscard]] auto IsExhausted() const noexcept -> bool;

private:
  class Implementation;

  std::unique_ptr<Implementation> implementation_;
};

} // namespace ggems::validation::random
