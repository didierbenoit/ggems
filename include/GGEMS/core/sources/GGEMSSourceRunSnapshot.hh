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

[[nodiscard]] auto BuildSourceRunSnapshot(GGEMSSource const &source)
    -> GGEMSSourceRunSnapshot;

[[nodiscard]] auto
BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources)
    -> GGEMSSourceRunSnapshot;

class GGEMSSourceRunSnapshot {
public:
  ~GGEMSSourceRunSnapshot() = default;

  GGEMSSourceRunSnapshot(GGEMSSourceRunSnapshot const &) = default;
  GGEMSSourceRunSnapshot(GGEMSSourceRunSnapshot &&) = default;
  auto operator=(GGEMSSourceRunSnapshot const &)
      -> GGEMSSourceRunSnapshot & = default;
  auto operator=(GGEMSSourceRunSnapshot &&)
      -> GGEMSSourceRunSnapshot & = default;

  [[nodiscard]] auto GetRecords() const noexcept
      -> std::vector<GGEMSSourceRecord> const & {
    return records_;
  }

  [[nodiscard]] auto GetRanges() const noexcept
      -> std::vector<GGEMSSourceRunRange> const & {
    return ranges_;
  }

  [[nodiscard]] auto GetTotalPrimaryCount() const noexcept -> std::uint64_t {
    return total_primary_count_;
  }

private:
  friend auto BuildSourceRunSnapshot(GGEMSSource const &source)
      -> GGEMSSourceRunSnapshot;

  friend auto
  BuildSourceRunSnapshot(std::span<std::shared_ptr<GGEMSSource> const> sources)
      -> GGEMSSourceRunSnapshot;

  GGEMSSourceRunSnapshot(std::vector<GGEMSSourceRecord> records,
                         std::vector<GGEMSSourceRunRange> ranges,
                         std::uint64_t total_primary_count);

  std::vector<GGEMSSourceRecord> records_;
  std::vector<GGEMSSourceRunRange> ranges_;
  std::uint64_t total_primary_count_{0ULL};
};
} // namespace ggems::core::sources
