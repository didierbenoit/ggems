#pragma once

#include <cstdint>
#include <memory>
#include <span>
#include <vector>

#include "GGEMS/core/sources/GGEMSSourceRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRunRange.hh"

namespace ggems::core::sources {

class GGEMSSource;
class GGEMSSourceRunSnapshot;

[[nodiscard]] GGEMSSourceRunSnapshot
BuildSourceRunSnapshot(GGEMSSource const &source);

[[nodiscard]] GGEMSSourceRunSnapshot
BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources);

class GGEMSSourceRunSnapshot {
public:
  ~GGEMSSourceRunSnapshot() = default;

  GGEMSSourceRunSnapshot(GGEMSSourceRunSnapshot const &) = default;
  GGEMSSourceRunSnapshot(GGEMSSourceRunSnapshot &&) = default;
  GGEMSSourceRunSnapshot &operator=(GGEMSSourceRunSnapshot const &) = default;
  GGEMSSourceRunSnapshot &operator=(GGEMSSourceRunSnapshot &&) = default;

public:
  [[nodiscard]] std::vector<GGEMSSourceRecord> const &
  GetRecords() const noexcept {
    return records_;
  }

  [[nodiscard]] std::vector<GGEMSSourceRunRange> const &
  GetRanges() const noexcept {
    return ranges_;
  }

  [[nodiscard]] std::uint64_t GetTotalPrimaryCount() const noexcept {
    return total_primary_count_;
  }

private:
  friend GGEMSSourceRunSnapshot
  BuildSourceRunSnapshot(GGEMSSource const &source);

  friend GGEMSSourceRunSnapshot
  BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources);

  GGEMSSourceRunSnapshot(std::vector<GGEMSSourceRecord> records,
                         std::vector<GGEMSSourceRunRange> ranges,
                         std::uint64_t total_primary_count);

private:
  std::vector<GGEMSSourceRecord> records_;
  std::vector<GGEMSSourceRunRange> ranges_;
  std::uint64_t total_primary_count_{0ULL};
};
} // namespace ggems::core::sources
