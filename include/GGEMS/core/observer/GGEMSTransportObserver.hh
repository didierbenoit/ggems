#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "GGEMS/core/observer/GGEMSObserverRecord.hh"

namespace ggems::core::observer {

class GGEMSTransportObserver {
public:
  GGEMSTransportObserver();
  ~GGEMSTransportObserver() = default;

  GGEMSTransportObserver(GGEMSTransportObserver const &) = delete;
  GGEMSTransportObserver(GGEMSTransportObserver &&) = delete;
  auto operator=(GGEMSTransportObserver const &)
      -> GGEMSTransportObserver & = delete;
  auto operator=(GGEMSTransportObserver &&)
      -> GGEMSTransportObserver & = delete;

  auto Enable(bool enabled = true) noexcept -> GGEMSTransportObserver &;
  auto Disable() noexcept -> GGEMSTransportObserver &;

  auto SetRecordCapacity(std::uint32_t record_capacity)
      -> GGEMSTransportObserver &;
  auto SetMaxStoredRecordCount(std::uint32_t max_stored_record_count)
      -> GGEMSTransportObserver &;

  auto CaptureFirstPrimaries(std::uint32_t primary_count) noexcept
      -> GGEMSTransportObserver &;

  auto CapturePrimary(std::uint64_t global_primary_id) noexcept
      -> GGEMSTransportObserver &;
  auto ClearCapturedPrimary() noexcept -> GGEMSTransportObserver &;

  void Clear();

  void Accumulate(std::span<GGEMSObserverRecord const> records,
                  GGEMSObserverCounters const &counters);

  [[nodiscard]] auto BuildConfigRecord() const noexcept
      -> GGEMSObserverConfigRecord;

  [[nodiscard]] auto IsEnabled() const noexcept -> bool;
  [[nodiscard]] auto GetRecordCapacity() const noexcept -> std::uint32_t;
  [[nodiscard]] auto GetRecordCount() const noexcept -> std::uint32_t;
  [[nodiscard]] auto GetOverflowCount() const noexcept -> std::uint32_t;
  [[nodiscard]] auto GetCapturedPrimaryCount() const noexcept -> std::uint32_t;

  [[nodiscard]] auto GetRecords() const noexcept
      -> std::vector<GGEMSObserverRecord> const &;

  [[nodiscard]] auto BuildDump(std::uint32_t max_record_count = 128U) const
      -> std::string;
  auto Verbose(std::uint32_t max_record_count = 128U) const -> void;

private:
  bool enabled_{false};

  std::uint32_t record_capacity_{4096U};
  std::uint32_t max_stored_record_count_{65'536U};

  std::uint32_t capture_first_primary_count_{0U};
  bool capture_specific_primary_enabled_{false};
  std::uint64_t capture_global_primary_id_{0xFFFFFFFFFFFFFFFFULL};

  GGEMSObserverCounters counters_{};
  std::vector<GGEMSObserverRecord> records_;
};

} // namespace ggems::core::observer
