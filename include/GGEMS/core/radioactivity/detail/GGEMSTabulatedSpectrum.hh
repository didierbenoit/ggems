#pragma once

#include <cstdint>
#include <span>

#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity::detail {

struct TabulatedSpectrumGrid {
  std::uint64_t lower_edge_milli_eV{0ULL};
  std::uint64_t bin_width_milli_eV{0ULL};
};

[[nodiscard]] auto
BuildTabulatedSpectrum(TabulatedSpectrumGrid grid,
                       std::span<double const> relative_bin_weights)
    -> sources::GGEMSEnergyDistribution;

} // namespace ggems::core::radioactivity::detail
