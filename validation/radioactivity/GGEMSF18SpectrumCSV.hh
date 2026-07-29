#pragma once

#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <iosfwd>
#include <span>
#include <string_view>

#include "GGEMS/core/sources/GGEMSEnergyDistribution.hh"

namespace ggems::validation::radioactivity {

struct GGEMSF18SpectrumCSVResult {
  std::size_t row_count{0U};
  std::uint64_t final_cumulative_ticket_upper_bound{0ULL};
};

[[nodiscard]] auto
WriteF18SpectrumCSV(std::ostream &output,
                    core::sources::GGEMSEnergyDistribution const &distribution)
    -> GGEMSF18SpectrumCSVResult;

[[nodiscard]] auto
ExportF18SpectrumCSV(std::filesystem::path const &output_path,
                     core::sources::GGEMSEnergyDistribution const &distribution)
    -> GGEMSF18SpectrumCSVResult;

[[nodiscard]] auto
RunF18SpectrumValidationCLI(std::span<std::string_view const> arguments,
                            std::ostream &standard_output,
                            std::ostream &standard_error) -> int;

} // namespace ggems::validation::radioactivity
