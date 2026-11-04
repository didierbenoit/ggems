#pragma once

#include <cstdint>
#include <filesystem>
#include <vector>

namespace ggems::ui::detail {

[[nodiscard]] auto ReadSPIRVFile(std::filesystem::path const &path)
  -> std::vector<std::uint32_t>;

} // namespace ggems::ui::detail
