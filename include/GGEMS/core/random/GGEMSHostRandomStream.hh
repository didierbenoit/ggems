#pragma once

#include <cstdint>
#include <variant>

#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/random/GGEMSRandomState.hh"
#include "GGEMS/core/random/GGEMSRandomEngine.hh"

namespace ggems::core::random {

class GGEMSHostRandomStream {
public:
  GGEMSHostRandomStream(GGEMSRandom const &random, std::uint64_t stream_id);

  [[nodiscard]] auto GetEngine() const noexcept -> GGEMSRandomEngine;
  [[nodiscard]] auto GetStreamId() const noexcept -> std::uint64_t;

  auto NextUInt32() noexcept -> std::uint32_t;
  auto UniformFloat01() noexcept -> float;
  auto UniformDoubleOpen01() noexcept -> double;

private:
  GGEMSRandomEngine engine_;
  std::uint64_t stream_id_;
  std::variant<GGEMSJKissState, GGEMSPCG32State, GGEMSPhiloxState> state_;
};

} // namespace ggems::core::random
