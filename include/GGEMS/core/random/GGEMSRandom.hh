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

public:
  GGEMSRandom &SetEngine(GGEMSRandomEngine engine) noexcept;
  GGEMSRandom &SetEngine(std::string_view engine_name);

  GGEMSRandomEngine GetEngine() const noexcept;
  std::string GetEngineName() const;

  GGEMSRandom &SetSeed(std::uint64_t seed) noexcept;
  std::uint64_t GetSeed() const noexcept;

  std::uint32_t GetKernelEngineId() const noexcept;
  std::string GetKernelBuildDefinition() const;

  std::size_t GetStateSize() const noexcept;

  void ValidateStateRange(std::uint64_t first_stream_id,
                          std::size_t state_storage) const;

  void InitialiseStates(std::uint64_t first_stream_id,
                        std::span<std::byte> state_storage) const;

  std::vector<std::string> BuildSummaryLines() const;
  void Verbose() const;

private:
  GGEMSRandomEngine engine_{GGEMSRandomEngine::JKISS};
  std::uint64_t seed_{77777ULL};
};

} // namespace ggems::core::random
