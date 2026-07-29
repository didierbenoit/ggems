#pragma once

#include <cstddef>
#include <cstdint>

#include "GGEMS/core/radioactivity/GGEMSBetaTransition.hh"
#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"

namespace ggems::core::radioactivity {

enum class GGEMSBetaSpectrumModel : std::uint8_t {
  BarePhaseSpaceDiagnostic,
  AllowedPointCoulomb
};

struct GGEMSBetaGridPolicy {
  std::uint64_t target_maximum_bin_width_milli_eV{500'000ULL};
};

struct GGEMSBetaSpectrumBuildOptions {
  GGEMSBetaSpectrumModel model{GGEMSBetaSpectrumModel::AllowedPointCoulomb};
  GGEMSBetaGridPolicy grid{};
};

struct GGEMSBetaSpectrumDiagnostics {
  GGEMSBetaSpectrumModel model{GGEMSBetaSpectrumModel::AllowedPointCoulomb};

  std::uint64_t lower_edge_milli_eV{0ULL};
  std::uint64_t upper_edge_milli_eV{0ULL};
  std::uint64_t bin_width_milli_eV{0ULL};
  std::size_t bin_count{0U};

  long double full_unnormalized_integral{0.0L};
  long double represented_unnormalized_integral{0.0L};
  long double excluded_probability{0.0L};

  long double continuous_mean_energy_milli_eV{0.0L};
  long double represented_mean_energy_milli_eV{0.0L};
  long double normalization_residual{0.0L};

  long double minimum_positive_bin_probability{0.0L};
  std::uint64_t minimum_assigned_ticket_count{0ULL};
};

struct GGEMSBetaSpectrumBuildResult {
  sources::GGEMSEnergyDistribution distribution;
  GGEMSBetaSpectrumDiagnostics diagnostics;
};

[[nodiscard]] auto
BuildBetaSpectrum(GGEMSBetaTransition const &transition,
                  GGEMSBetaSpectrumBuildOptions const &options = {})
    -> GGEMSBetaSpectrumBuildResult;
} // namespace ggems::core::radioactivity
