#pragma once

#include <string_view>

namespace ggems::ocl {

enum class SVMMemoryKind {
  None,
  Auto,
  CoarseGrainBuffer,
  FineGrainBuffer,
  FineGrainBufferAtomics,
  FineGrainSystem
};

[[nodiscard]] constexpr std::string_view ToString(SVMMemoryKind kind) noexcept {
  switch (kind) {
  case SVMMemoryKind::None:
    return "None";
  case SVMMemoryKind::Auto:
    return "Auto";
  case SVMMemoryKind::CoarseGrainBuffer:
    return "CoarseGrainBuffer";
  case SVMMemoryKind::FineGrainBuffer:
    return "FineGrainBuffer";
  case SVMMemoryKind::FineGrainBufferAtomics:
    return "FineGrainBufferAtomics";
  case SVMMemoryKind::FineGrainSystem:
    return "FineGrainSystem";
  }

  return "Unknown";
}

[[nodiscard]] constexpr bool RequiresExplicitMap(SVMMemoryKind kind) noexcept {
  return kind == SVMMemoryKind::CoarseGrainBuffer;
}

} // namespace ggems::ocl
