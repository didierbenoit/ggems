#include <cstdint>
#include <string_view>

#include "GGEMS/frameworks/GGEMSOpenCLCacheFingerprint.hh"

namespace ggems::ocl::detail {
namespace {
constexpr std::uint64_t k_fnv1a64_offset_basis{14695981039346656037ULL};
constexpr std::uint64_t k_fnv1a64_prime{1099511628211ULL};
} // namespace

[[nodiscard]] auto HashFNV1a64(std::string_view bytes) noexcept
    -> std::uint64_t {
  auto hash = k_fnv1a64_offset_basis;

  for (char const byte : bytes) {
    auto const unsigned_byte = static_cast<unsigned char>(byte);
    hash ^= static_cast<std::uint64_t>(unsigned_byte);
    hash *= k_fnv1a64_prime;
  }

  return hash;
}

} // namespace ggems::ocl::detail
