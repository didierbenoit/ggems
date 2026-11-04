#include <cstddef>
#include <cstdint>
#include <format>
#include <fstream>
#include <ios>
#include <vector>
#include <filesystem>

#include "GGEMS/ui/detail/GGEMSVulkanShaderFile.hh"
#include "GGEMS/GGEMSException.hh"

namespace ggems::ui::detail {

auto ReadSPIRVFile(std::filesystem::path const &path)
  -> std::vector<std::uint32_t> {
  std::ifstream file{path, std::ios::binary | std::ios::ate};

  if (!file.is_open()) {
    throw ggems::core::GGEMSInternal(
      std::format("Unable to open SPIR-V shader file '{}'.", path.string()));
  }

  std::streamsize file_size = file.tellg();

  if (!(file_size > 0)) {
    throw ggems::core::GGEMSInternal(
      std::format("SPIR-V shader file '{}' is empty.", path.string()));
  }

  if (!(file_size % static_cast<std::streamsize>(sizeof(std::uint32_t)) == 0)) {
    throw ggems::core::GGEMSInternal(std::format(
      "SPIR-V shader file '{}' has an invalid byte size.", path.string()));
  }

  file.seekg(0, std::ios::beg);

  std::size_t word_count =
    static_cast<std::size_t>(file_size) / sizeof(std::uint32_t);

  std::vector<std::uint32_t> code(word_count);

  file.read(reinterpret_cast<char *>(code.data()), file_size);

  if (!file.good()) {
    throw ggems::core::GGEMSInternal(
      std::format("Unable to read SPIR-V shader file '{}'.", path.string()));
  }

  return code;
}

} // namespace ggems::ui::detail
