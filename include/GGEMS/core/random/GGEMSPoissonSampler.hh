#pragma once

#include <cstdint>

namespace ggems::core::random {

class GGEMSHostRandomStream;

[[nodiscard]] auto SamplePoisson(long double mean,
                                 GGEMSHostRandomStream &random)
    -> std::uint64_t;
} // namespace ggems::core::random
