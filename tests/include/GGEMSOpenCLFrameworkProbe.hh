#pragma once

#include <string>
#include <filesystem>

namespace ggems::test {

inline const std::string k_opencl_framework_probe_name{
    "opencl_framework_probe"};

[[nodiscard]] inline auto GetOpenCLFrameworkProbeRoot()
    -> std::filesystem::path {
  return std::filesystem::path{GGEMS_TEST_KERNEL_ROOT} / "tests";
}
} // namespace ggems::test
