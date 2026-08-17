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
  if (grid.lower_edge_milli_eV == 0ULL || grid.bin_width_milli_eV == 0ULL ||
      (grid.bin_width_milli_eV & 1ULL) != 0ULL) {
    throw GGEMSInternal("Tabulated spectrum grid is invalid.");
  }

  std::uint64_t const half_width = grid.bin_width_milli_eV / 2ULL;

  if (grid.lower_edge_milli_eV >
      std::numeric_limits<std::uint64_t>::max() - half_width) {
    throw GGEMSInternal("Tabulated spectrum first center overflows.");
  }

  std::uint64_t center = grid.lower_edge_milli_eV + half_width;

  std::vector<double> centers;
  centers.reserve(relative_bin_weights.size());
  constexpr std::uint64_t k_max_exact_double_integer{
      std::uint64_t{1ULL} << std::numeric_limits<double>::digits};

  for (std::size_t index = 0U; index < relative_bin_weights.size(); ++index) {
    if (center > k_max_exact_double_integer) {
      throw GGEMSInternal(
          "Tabulated spectrum center is not exactly representable as "
          "double.");
    }

    centers.push_back(static_cast<double>(center));

    if (index + 1U < relative_bin_weights.size()) {
      if (center >
          std::numeric_limits<std::uint64_t>::max() - grid.bin_width_milli_eV) {
        throw GGEMSInternal("Tabulated spectrum center overflows.");
      }
      center += grid.bin_width_milli_eV;
    }
  }

  return sources::GGEMSEnergyDistribution::BuildRegularSpectrum(
      centers, relative_bin_weights, "meV");
}

} // namespace ggems::core::radioactivity::detail
