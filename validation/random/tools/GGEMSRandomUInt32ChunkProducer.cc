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
 * \brief Implements validation-private OpenCL raw uint32 chunk production.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#include "GGEMSRandomUInt32ChunkProducer.hh"

/// \cond
#include <algorithm>
#include <climits>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <format>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

/// \endcond

#include "GGEMS/opencl/GGEMSOpenCL.hh"
#include "GGEMS/opencl/GGEMSOpenCLContext.hh"
#include "GGEMS/opencl/GGEMSOpenCLDevice.hh"
#include "GGEMS/opencl/GGEMSOpenCLKernel.hh"
#include "GGEMS/opencl/GGEMSOpenCLLaunchGeometry.hh"
#include "GGEMS/opencl/GGEMSOpenCLProgram.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMBuffer.hh"
#include "GGEMS/opencl/GGEMSOpenCLSVMHostAccess.hh"
#include "GGEMS/random/GGEMSRandom.hh"
#include "GGEMS/units/GGEMSBytesUnits.hh"

namespace {

// =============================================================================
// =============================================================================

constexpr std::uint64_t k_bytes_per_word{sizeof(std::uint32_t)};
constexpr int k_required_bits_per_byte{8};

static_assert(CHAR_BIT == k_required_bits_per_byte);
static_assert(sizeof(std::uint32_t) == 4U);

[[nodiscard]] auto CheckedMultiply(std::uint64_t lhs, std::uint64_t rhs,
                                   std::string_view label) -> std::uint64_t {
  if (lhs != 0ULL && rhs > std::numeric_limits<std::uint64_t>::max() / lhs) {
    throw std::runtime_error(std::format("{} overflows uint64.", label));
  }
  return lhs * rhs;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto DivideRoundUp(std::uint64_t numerator,
                                 std::uint64_t denominator) -> std::uint64_t {
  return (numerator / denominator) +
         static_cast<std::uint64_t>((numerator % denominator) != 0ULL);
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto ComputeChunkPlan(
    ggems::validation::random::RandomUInt32StreamSpecification const
        &specification,
    ggems::core::random::GGEMSRandom const &random,
    ggems::ocl::GGEMSOpenCLDevice const &device)
    -> ggems::validation::random::RandomUInt32ChunkPlan {
  using ggems::validation::random::RandomUInt32ChunkPlan;
  using ggems::validation::random::RandomUInt32ChunkStrategy;
  using ggems::validation::random::RandomUInt32StreamLayout;

  if (specification.worker_count == 0U ||
      specification.samples_per_worker == 0U) {
    throw std::runtime_error(
        "Raw uint32 chunk planning requires nonzero stream dimensions.");
  }

  RandomUInt32ChunkPlan plan;
  plan.requested_max_value_buffer_size =
      specification.maximum_value_buffer_size;
  plan.device_max_allocation_size =
      ggems::units::Bytes{device.GetMaxMemAllocSize()};
  plan.effective_max_value_buffer_size =
      ggems::units::Bytes{std::min(plan.requested_max_value_buffer_size.value,
                                   plan.device_max_allocation_size.value)};

  if (plan.effective_max_value_buffer_size.value == 0ULL) {
    throw std::runtime_error(
        "OpenCL device reports a zero effective maximum allocation size.");
  }

  auto const state_size = static_cast<std::uint64_t>(random.GetStateSize());
  if (state_size == 0ULL) {
    throw std::runtime_error("Random engine reports a zero state size.");
  }

  if (specification.layout == RandomUInt32StreamLayout::Interleaved) {
    plan.strategy = RandomUInt32ChunkStrategy::SampleDepth;
    plan.workers_per_chunk = specification.worker_count;
    plan.state_buffer_size = ggems::units::Bytes{
        CheckedMultiply(specification.worker_count, state_size,
                        "Random state buffer byte count")};

    if (plan.state_buffer_size.value > plan.device_max_allocation_size.value) {
      throw std::runtime_error(
          "Random state buffer exceeds CL_DEVICE_MAX_MEM_ALLOC_SIZE.");
    }

    std::uint64_t const bytes_per_sample_round =
        CheckedMultiply(specification.worker_count, k_bytes_per_word,
                        "Interleaved sample-round byte count");
    std::uint64_t const maximum_depth =
        plan.effective_max_value_buffer_size.value / bytes_per_sample_round;
    if (maximum_depth == 0ULL) {
      throw std::runtime_error(
          "Maximum chunk size is too small for one interleaved sample round.");
    }

    plan.samples_per_worker_per_chunk =
        static_cast<std::uint32_t>(std::min<std::uint64_t>(
            specification.samples_per_worker, maximum_depth));
    plan.value_buffer_size = ggems::units::Bytes{
        CheckedMultiply(CheckedMultiply(specification.worker_count,
                                        plan.samples_per_worker_per_chunk,
                                        "Interleaved chunk word count"),
                        k_bytes_per_word, "Interleaved chunk byte count")};
    plan.chunk_count = DivideRoundUp(specification.samples_per_worker,
                                     plan.samples_per_worker_per_chunk);
    return plan;
  }

  plan.strategy = RandomUInt32ChunkStrategy::WorkerGroups;
  plan.samples_per_worker_per_chunk = specification.samples_per_worker;
  std::uint64_t const value_bytes_per_worker =
      CheckedMultiply(specification.samples_per_worker, k_bytes_per_word,
                      "Worker-major worker byte count");
  std::uint64_t const maximum_workers_by_values =
      plan.effective_max_value_buffer_size.value / value_bytes_per_worker;
  std::uint64_t const maximum_workers_by_states =
      plan.device_max_allocation_size.value / state_size;
  std::uint64_t const maximum_workers =
      std::min(maximum_workers_by_values, maximum_workers_by_states);
  if (maximum_workers == 0ULL) {
    throw std::runtime_error(
        "Maximum chunk size is too small for one worker-major stream.");
  }

  plan.workers_per_chunk = static_cast<std::uint32_t>(
      std::min<std::uint64_t>(specification.worker_count, maximum_workers));
  plan.state_buffer_size = ggems::units::Bytes{
      CheckedMultiply(plan.workers_per_chunk, state_size,
                      "Worker-major state buffer byte count")};
  plan.value_buffer_size = ggems::units::Bytes{
      CheckedMultiply(plan.workers_per_chunk, value_bytes_per_worker,
                      "Worker-major value buffer byte count")};
  plan.chunk_count =
      DivideRoundUp(specification.worker_count, plan.workers_per_chunk);
  return plan;
}

} // namespace

// ============================================================================
// ============================================================================

namespace ggems::validation::random {

auto ToString(RandomUInt32StreamLayout layout) -> std::string_view {
  switch (layout) {
  case RandomUInt32StreamLayout::WorkerMajor:
    return "worker_major";
  case RandomUInt32StreamLayout::Interleaved:
    return "interleaved";
  }
  throw std::runtime_error("Unsupported random uint32 stream layout.");
}

// ----------------------------------------------------------------------------

auto ParseRandomUInt32StreamLayout(std::string_view value)
    -> RandomUInt32StreamLayout {
  if (value == "worker_major") {
    return RandomUInt32StreamLayout::WorkerMajor;
  }
  if (value == "interleaved") {
    return RandomUInt32StreamLayout::Interleaved;
  }
  throw std::runtime_error(
      std::format("Unsupported random uint32 stream layout '{}'.", value));
}

// ----------------------------------------------------------------------------

auto ToString(RandomUInt32ChunkStrategy strategy) -> std::string_view {
  switch (strategy) {
  case RandomUInt32ChunkStrategy::SampleDepth:
    return "sample_depth";
  case RandomUInt32ChunkStrategy::WorkerGroups:
    return "worker_groups";
  }
  throw std::runtime_error("Unsupported random uint32 chunk strategy.");
}

// ============================================================================
// ============================================================================

class GGEMSRandomUInt32ChunkProducer::Implementation {
public:
  Implementation(RandomUInt32StreamSpecification specification,
                 ggems::ocl::GGEMSOpenCLContext &context,
                 std::optional<std::uint64_t> output_word_limit)
      : specification_{specification}, context_{context} {
    ValidateSpecification();

    random_.SetEngine(specification_.engine);
    random_.SetSeed(specification_.seed);
    random_.ValidateStateRange(specification_.stream_offset,
                               specification_.worker_count);

    total_word_count_ = CheckedMultiply(specification_.worker_count,
                                        specification_.samples_per_worker,
                                        "Logical raw uint32 word count");
    static_cast<void>(CheckedMultiply(total_word_count_, k_bytes_per_word,
                                      "Logical raw uint32 byte count"));

    output_word_limit_ = output_word_limit.value_or(total_word_count_);
    if (output_word_limit_ > total_word_count_) {
      throw std::runtime_error(
          "Output word limit exceeds the finite logical stream capacity.");
    }

    auto const &device = context_.GetDevice();
    chunk_plan_ = ComputeChunkPlan(specification_, random_, device);
    CreateKernelAndBuffers();

    if (specification_.layout == RandomUInt32StreamLayout::Interleaved &&
        output_word_limit_ != 0ULL) {
      InitializeStates({
          .first_stream_id = specification_.stream_offset,
          .worker_count = specification_.worker_count,
      });
    }
  }

  [[nodiscard]] auto NextChunk() -> std::span<std::uint32_t const> {
    if (returned_word_count_ == output_word_limit_) {
      return {};
    }

    std::uint64_t generated_this_chunk = 0ULL;
    if (specification_.layout == RandomUInt32StreamLayout::Interleaved) {
      std::uint32_t const remaining_depth =
          specification_.samples_per_worker - next_sample_depth_;
      std::uint32_t const current_depth =
          std::min(chunk_plan_.samples_per_worker_per_chunk, remaining_depth);
      RunKernel(specification_.worker_count, current_depth);
      generated_this_chunk =
          CheckedMultiply(specification_.worker_count, current_depth,
                          "Generated interleaved chunk word count");
      next_sample_depth_ += current_depth;
    } else {
      std::uint32_t const current_worker_count =
          std::min(chunk_plan_.workers_per_chunk,
                   specification_.worker_count - next_worker_index_);
      std::uint64_t const first_stream_id =
          specification_.stream_offset + next_worker_index_;
      InitializeStates({
          .first_stream_id = first_stream_id,
          .worker_count = current_worker_count,
      });
      RunKernel(current_worker_count, specification_.samples_per_worker);
      generated_this_chunk = CheckedMultiply(
          current_worker_count, specification_.samples_per_worker,
          "Generated worker-major chunk word count");
      next_worker_index_ += current_worker_count;
    }


    if (generated_this_chunk > host_values_.size()) {
      throw std::runtime_error(
          "Generated chunk exceeds the bounded host staging buffer.");
    }
    auto generated_values = std::span{host_values_}.first(
        static_cast<std::size_t>(generated_this_chunk));
    ggems::ocl::ReadSVMToHost(*values_buffer_, generated_values);

    std::uint64_t const remaining_limit =
        output_word_limit_ - returned_word_count_;
    std::uint64_t const returned_this_chunk =
        std::min(generated_this_chunk, remaining_limit);
    returned_word_count_ += returned_this_chunk;
    return std::span<std::uint32_t const>{host_values_}.first(
        static_cast<std::size_t>(returned_this_chunk));
  }

  [[nodiscard]] auto GetChunkPlan() const noexcept
      -> RandomUInt32ChunkPlan const & {
    return chunk_plan_;
  }

  [[nodiscard]] auto GetReturnedWordCount() const noexcept -> std::uint64_t {
    return returned_word_count_;
  }

  [[nodiscard]] auto IsExhausted() const noexcept -> bool {
    return returned_word_count_ == output_word_limit_;
  }

private:
  struct StateInitializationRequest {
    std::uint64_t first_stream_id;
    std::uint32_t worker_count;
  };

  auto ValidateSpecification() const -> void {
    switch (specification_.layout) {
    case RandomUInt32StreamLayout::WorkerMajor:
    case RandomUInt32StreamLayout::Interleaved:
      break;
    default:
      throw std::runtime_error("Unsupported random uint32 stream layout.");
    }
    if (specification_.worker_count == 0U) {
      throw std::runtime_error("Worker count must be greater than zero.");
    }
    if (specification_.samples_per_worker == 0U) {
      throw std::runtime_error("Samples per worker must be greater than zero.");
    }
    if (specification_.local_size == 0U) {
      throw std::runtime_error("Local size must be greater than zero.");
    }
    if (specification_.maximum_value_buffer_size.value == 0ULL) {
      throw std::runtime_error(
          "Maximum value-buffer size must be greater than zero.");
    }
  }

  auto CreateKernelAndBuffers() -> void {
    std::filesystem::path const kernel_root{GGEMS_KERNEL_ROOT};
    std::filesystem::path const validation_kernel_root{
        GGEMS_VALIDATION_RANDOM_KERNEL_ROOT};
    std::string const requested_build_options =
        std::format("-I\"{}\" {}", kernel_root.generic_string(),
                    random_.GetKernelBuildDefinition());

    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();
    auto const &program = opencl.GetOrCreateProgram(
        context_, validation_kernel_root, "random_uint32_stream",
        requested_build_options);
    kernel_ = std::make_unique<ggems::ocl::GGEMSOpenCLKernel>(
        context_, program.CreateKernel("random_uint32_stream"),
        "random_uint32_stream");

    if (chunk_plan_.state_buffer_size.value >
            std::numeric_limits<std::size_t>::max() ||
        chunk_plan_.value_buffer_size.value >
            std::numeric_limits<std::size_t>::max()) {
      throw std::runtime_error(
          "Chunk staging allocation exceeds this host's size_t domain.");
    }

    states_buffer_ = std::make_unique<ggems::ocl::GGEMSOpenCLSVMBuffer>(
        context_.CreateSVMBuffer(chunk_plan_.state_buffer_size));
    values_buffer_ = std::make_unique<ggems::ocl::GGEMSOpenCLSVMBuffer>(
        context_.CreateSVMBuffer(chunk_plan_.value_buffer_size));

    host_states_.resize(
        static_cast<std::size_t>(chunk_plan_.state_buffer_size.value));
    host_values_.resize(static_cast<std::size_t>(
        chunk_plan_.value_buffer_size.value / k_bytes_per_word));
  }

  auto InitializeStates(StateInitializationRequest const &request) -> void {
    std::uint64_t const state_bytes =
        CheckedMultiply(request.worker_count,
                        static_cast<std::uint64_t>(random_.GetStateSize()),
                        "Random state chunk byte count");
    if (state_bytes > host_states_.size()) {
      throw std::runtime_error(
          "Random state chunk exceeds the bounded host staging buffer.");
    }

    auto state_storage =
        std::span{host_states_}.first(static_cast<std::size_t>(state_bytes));
    random_.InitializeStates(request.first_stream_id, state_storage);
    ggems::ocl::WriteSVMFromHost(*states_buffer_,
                                 std::span<std::byte const>{state_storage});
  }

  auto RunKernel(std::uint32_t worker_count, std::uint32_t samples_per_worker)
      -> void {
    kernel_->SetArgSVMPointer(0U, states_buffer_->GetData());
    kernel_->SetArgSVMPointer(1U, values_buffer_->GetData());
    kernel_->SetArg(2U, worker_count);
    kernel_->SetArg(3U, samples_per_worker);
    kernel_->SetArg(4U, static_cast<std::uint32_t>(specification_.layout));

    auto const padded_global_work_size =
        ggems::ocl::detail::TryComputePaddedGlobalWorkSize(
            static_cast<std::size_t>(worker_count), specification_.local_size);
    if (!padded_global_work_size.has_value()) {
      throw std::runtime_error(
          "Unable to compute the padded OpenCL global work size.");
    }
    kernel_->Run({*padded_global_work_size}, {specification_.local_size});

  }

  RandomUInt32StreamSpecification specification_;
  ggems::ocl::GGEMSOpenCLContext &context_;
  ggems::core::random::GGEMSRandom random_;
  RandomUInt32ChunkPlan chunk_plan_;
  std::unique_ptr<ggems::ocl::GGEMSOpenCLKernel> kernel_;
  std::unique_ptr<ggems::ocl::GGEMSOpenCLSVMBuffer> states_buffer_;
  std::unique_ptr<ggems::ocl::GGEMSOpenCLSVMBuffer> values_buffer_;
  std::vector<std::byte> host_states_;
  std::vector<std::uint32_t> host_values_;
  std::uint64_t total_word_count_{0ULL};
  std::uint64_t output_word_limit_{0ULL};
  std::uint64_t returned_word_count_{0ULL};
  std::uint32_t next_sample_depth_{0U};
  std::uint32_t next_worker_index_{0U};
};

// ============================================================================
// ============================================================================

GGEMSRandomUInt32ChunkProducer::GGEMSRandomUInt32ChunkProducer(
    RandomUInt32StreamSpecification specification,
    ggems::ocl::GGEMSOpenCLContext &context,
    std::optional<std::uint64_t> output_word_limit)
    : implementation_{std::make_unique<Implementation>(
          specification, context, output_word_limit)} {}

GGEMSRandomUInt32ChunkProducer::~GGEMSRandomUInt32ChunkProducer() = default;

// ----------------------------------------------------------------------------

auto GGEMSRandomUInt32ChunkProducer::NextChunk()
    -> std::span<std::uint32_t const> {
  return implementation_->NextChunk();
}

// ----------------------------------------------------------------------------

auto GGEMSRandomUInt32ChunkProducer::GetChunkPlan() const noexcept
    -> RandomUInt32ChunkPlan const & {
  return implementation_->GetChunkPlan();
}

// ----------------------------------------------------------------------------

auto GGEMSRandomUInt32ChunkProducer::GetReturnedWordCount() const noexcept
    -> std::uint64_t {
  return implementation_->GetReturnedWordCount();
}

// ----------------------------------------------------------------------------

auto GGEMSRandomUInt32ChunkProducer::IsExhausted() const noexcept -> bool {
  return implementation_->IsExhausted();
}

} // namespace ggems::validation::random
