#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "GGEMS/radioactivity/detail/GGEMSTabulatedSpectrum.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity::detail {

// =============================================================================
// =============================================================================

[[nodiscard]] auto
BuildTabulatedSpectrum(TabulatedSpectrumGrid grid,
                       std::span<double const> relative_bin_weights)
  -> sources::GGEMSEnergyDistribution {
  std::uint64_t const first_center =
    grid.lower_edge_micro_eV + (grid.bin_width_micro_eV / 2ULL);

  std::vector<std::uint64_t> centers;
  centers.reserve(relative_bin_weights.size());

  for (std::size_t index = 0U; index < relative_bin_weights.size(); ++index) {
    centers.push_back(first_center + (static_cast<std::uint64_t>(index) *
                                      grid.bin_width_micro_eV));
  }

  return sources::GGEMSEnergyDistribution::BuildRegularSpectrum(
    centers, relative_bin_weights);
}

} // namespace ggems::core::radioactivity::detail
