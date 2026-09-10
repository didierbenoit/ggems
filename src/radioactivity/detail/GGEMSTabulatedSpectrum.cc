#include <cstddef>
#include <cstdint>
#include <limits>
#include <span>
#include <vector>

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/radioactivity/detail/GGEMSTabulatedSpectrum.hh"
#include "GGEMS/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity::detail {

// =============================================================================
// =============================================================================

[[nodiscard]] auto
BuildTabulatedSpectrum(TabulatedSpectrumGrid grid,
                       std::span<const double> relative_bin_weights)
    -> sources::GGEMSEnergyDistribution {
  if (grid.lower_edge_micro_eV == 0ULL || grid.bin_width_micro_eV == 0ULL ||
      (grid.bin_width_micro_eV & 1ULL) != 0ULL) {
    throw GGEMSInternal("Tabulated spectrum grid is invalid.");
  }

  std::uint64_t const half_width = grid.bin_width_micro_eV / 2ULL;

  if (grid.lower_edge_micro_eV >
      std::numeric_limits<std::uint64_t>::max() - half_width) {
    throw GGEMSInternal("Tabulated spectrum first center overflows.");
  }

  std::uint64_t center = grid.lower_edge_micro_eV + half_width;

  std::vector<std::uint64_t> centers;
  centers.reserve(relative_bin_weights.size());
  for (std::size_t index = 0U; index < relative_bin_weights.size(); ++index) {
    centers.push_back(center);

    if (index + 1U < relative_bin_weights.size()) {
      if (center >
          std::numeric_limits<std::uint64_t>::max() - grid.bin_width_micro_eV) {
        throw GGEMSInternal("Tabulated spectrum center overflows.");
      }
      center += grid.bin_width_micro_eV;
    }
  }

  return sources::GGEMSEnergyDistribution::BuildRegularSpectrum(
      centers, relative_bin_weights);
}

} // namespace ggems::core::radioactivity::detail
