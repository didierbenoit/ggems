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
 * \brief Declares validation-private OpenCL raw uint32 chunk production.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
/// \endcond

#include "GGEMS/random/GGEMSRandomEngine.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"

namespace ggems::ocl {
class GGEMSOpenCLContext;
}

/*!
 * \namespace ggems::validation::random
 * \brief Provides validation-only deterministic raw random-stream utilities.
 */
namespace ggems::validation::random {

/*!
 * \brief Selects the flattened ordering of worker streams.
 */
enum class RandomUInt32StreamLayout : std::uint8_t {
  WorkerMajor = 0U, /*!< Emits every requested sample from one worker first. */
  Interleaved = 1U  /*!< Emits one sample from every worker per depth round. */
};

/*!
 * \brief Returns the manifest spelling for a raw-stream layout.
 *
 * \param[in] layout Layout to format.
 * \return Stable layout spelling.
 */
[[nodiscard]] auto ToString(RandomUInt32StreamLayout layout)
    -> std::string_view;

/*!
 * \brief Parses a raw-stream layout spelling.
 *
 * \param[in] value Layout spelling.
 * \return Parsed layout.
 * \throws std::runtime_error If the spelling is unsupported.
 */
[[nodiscard]] auto ParseRandomUInt32StreamLayout(std::string_view value)
    -> RandomUInt32StreamLayout;

/*!
 * \brief Defines one finite logical GGEMS raw uint32 stream rectangle.
 *
 * Chunk capacity affects only execution partitioning. It is not part of the
 * flattened sequence definition.
 */
struct RandomUInt32StreamSpecification {
  ggems::core::random::GGEMSRandomEngine engine{
      ggems::core::random::GGEMSRandomEngine::Philox}; /*!< GGEMS engine. */
  std::uint64_t seed{77'777ULL};                       /*!< Full GGEMS seed. */
  std::uint64_t stream_offset{0ULL};    /*!< First logical stream ID. */
  std::uint32_t worker_count{1U};       /*!< Logical worker count. */
  std::uint32_t samples_per_worker{1U}; /*!< Logical depth per worker. */
  RandomUInt32StreamLayout layout{
      RandomUInt32StreamLayout::Interleaved}; /*!< Flattening layout. */
  std::size_t local_size{64U};                /*!< OpenCL local work size. */
  ggems::units::Bytes maximum_value_buffer_size{
      256ULL * 1024ULL * 1024ULL}; /*!< Requested value-buffer ceiling. */
};

/*!
 * \brief Identifies how the logical stream is partitioned into OpenCL chunks.
 */
enum class RandomUInt32ChunkStrategy : std::uint8_t {
  SampleDepth = 0U, /*!< Interleaved full-worker sample-depth chunks. */
  WorkerGroups      /*!< Worker-major complete-worker groups. */
};

/*!
 * \brief Returns the manifest spelling for a chunk strategy.
 *
 * \param[in] strategy Strategy to format.
 * \return Stable strategy spelling.
 */
[[nodiscard]] auto ToString(RandomUInt32ChunkStrategy strategy)
    -> std::string_view;

/*!
 * \brief Records the validated OpenCL allocation and chunk plan.
 */
struct RandomUInt32ChunkPlan {
  RandomUInt32ChunkStrategy strategy{
      RandomUInt32ChunkStrategy::SampleDepth}; /*!< Partitioning strategy. */
  ggems::units::Bytes requested_max_value_buffer_size{
      0ULL}; /*!< Requested cap. */
  ggems::units::Bytes effective_max_value_buffer_size{0ULL}; /*!< Device cap. */
  ggems::units::Bytes device_max_allocation_size{0ULL}; /*!< Allocation cap. */
  ggems::units::Bytes state_buffer_size{0ULL};    /*!< Allocated state bytes. */
  ggems::units::Bytes value_buffer_size{0ULL};    /*!< Allocated value bytes. */
  std::uint32_t workers_per_chunk{0U};            /*!< Maximum workers/chunk. */
  std::uint32_t samples_per_worker_per_chunk{0U}; /*!< Maximum depth/chunk. */
  std::uint64_t chunk_count{0ULL}; /*!< Full-domain chunk count. */
};

/*!
 * \brief Produces deterministic GGEMS raw uint32 values in bounded chunks.
 *
 * The object preserves mutable worker state across interleaved refills and
 * complete-worker ordering for worker-major output. Returned spans remain
 * valid only until the next call to NextChunk or object destruction.
 */
class GGEMSRandomUInt32ChunkProducer {
public:
  /*!
   * \brief Constructs a producer on one initialized OpenCL context.
   *
   * \param[in] specification Immutable logical stream specification.
   * \param[in,out] context Initialized context used for allocations and
   * kernels.
   * \param[in] output_word_limit Optional serialized-prefix limit. The limit
   *                              never redefines samples_per_worker.
   */
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

  /*!
   * \brief Generates and returns the next flattened numeric chunk.
   *
   * The final chunk is cropped to output_word_limit after device generation.
   *
   * \return Next chunk, or an empty span after the requested prefix is
   * complete.
   */
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
