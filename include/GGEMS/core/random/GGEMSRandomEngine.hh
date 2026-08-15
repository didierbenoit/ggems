#pragma once

#include <cstdint>
#include <string>
#include <string_view>

namespace ggems::core::random {

enum class GGEMSRandomEngine : std::uint8_t {
  JKISS = 1U,
  PCG32 = 2U,
  Philox = 3U
};

std::string ToString(GGEMSRandomEngine engine);

GGEMSRandomEngine ParseRandomEngine(std::string_view engine_name);

std::uint32_t ToKernelEngineId(GGEMSRandomEngine engine) noexcept;
} // namespace ggems::core::random
