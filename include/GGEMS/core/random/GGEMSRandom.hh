#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "GGEMS/core/random/GGEMSRandomEngine.hh"

namespace ggems::core::random {

class GGEMSRandom {
public:
  GGEMSRandom();

  auto SetEngine(GGEMSRandomEngine engine) noexcept -> GGEMSRandom &;
  auto SetEngine(std::string_view engine_name) -> GGEMSRandom &;

  [[nodiscard]] auto GetEngine() const noexcept -> GGEMSRandomEngine;
  [[nodiscard]] auto GetEngineName() const -> std::string;

  auto SetSeed(std::uint64_t seed) noexcept -> GGEMSRandom &;
  [[nodiscard]] auto GetSeed() const noexcept -> std::uint64_t;

  [[nodiscard]] auto GetKernelEngineId() const noexcept -> std::uint32_t;
  [[nodiscard]] auto GetKernelBuildDefinition() const -> std::string;

  [[nodiscard]] auto GetStateSize() const noexcept -> std::size_t;

  auto ValidateStateRange(std::uint64_t first_stream_id,
                          std::size_t state_count) const -> void;

  auto InitializeStates(std::uint64_t first_stream_id,
                        std::span<std::byte> state_storage) const -> void;

  [[nodiscard]] auto BuildSummaryLines() const -> std::vector<std::string>;
  void Verbose() const;

private:
  GGEMSRandomEngine engine_{GGEMSRandomEngine::JKISS};
  std::uint64_t seed_{77777ULL};
};

} // namespace ggems::core::random
