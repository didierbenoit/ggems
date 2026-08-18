#pragma once

#include <cstdint>
#include <optional>
#include <string_view>
#include <span>
#include <string>

#include "GGEMS/radioactivity/GGEMSRadionuclideDefinition.hh"

namespace ggems::core::radioactivity::builtins {

struct GGEMSBuiltInRadionuclideInfo {
  std::string_view canonical_name;
  std::string_view element_name;
  std::uint32_t atomic_number;
  std::uint32_t mass_number;
  std::string_view daughter_name;
  std::string_view decay_mode;
  long double q_value_kilo_electron_volt;
  std::string_view nuclear_data_source;
  std::string_view beta_spectrum_source;
  std::string_view atomic_data_source;
  std::string_view model_notes;
};

[[nodiscard]] auto BuildH3Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildC14Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildF18Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildC11Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildO15Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildGa68Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildCo60Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildLu177Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildI131Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildAm241Radionuclide() -> GGEMSRadionuclideDefinition;
[[nodiscard]] auto BuildTc99mRadionuclide() -> GGEMSRadionuclideDefinition;

[[nodiscard]] auto GetAvailableRadionuclideNames() noexcept
    -> std::span<std::string_view const>;

[[nodiscard]] auto
FindBuiltInRadionuclideInfo(std::string_view canonical_name) noexcept
    -> GGEMSBuiltInRadionuclideInfo const *;

[[nodiscard]] auto BuildBuiltInRadionuclide(std::string_view canonical_name)
    -> std::optional<GGEMSRadionuclideDefinition>;

[[nodiscard]] auto
DescribeBuiltInRadionuclide(GGEMSRadionuclideDefinition const &definition)
    -> std::string;

[[nodiscard]] auto DescribeBuiltInRadionuclide(std::string_view canonical_name)
    -> std::string;

auto VerboseBuiltInRadionuclide(GGEMSRadionuclideDefinition const &definition)
    -> void;

auto VerboseBuiltInRadionuclide(std::string_view canonical_name) -> void;

} // namespace ggems::core::radioactivity::builtins
