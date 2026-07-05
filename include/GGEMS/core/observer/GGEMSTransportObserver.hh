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
  GGEMSTransportObserver &operator=(GGEMSTransportObserver const &) = delete;
  GGEMSTransportObserver &operator=(GGEMSTransportObserver &&) = delete;

public:
  GGEMSTransportObserver &Enable(bool enabled = true) noexcept;
  GGEMSTransportObserver &Disable() noexcept;

  GGEMSTransportObserver &SetRecordCapacity(std::uint32_t record_capacity);
  GGEMSTransportObserver &
  SetMaxStoredRecordCount(std::uint32_t max_stored_record_count);

  GGEMSTransportObserver &
  CaptureFirstPrimaries(std::uint32_t primary_count) noexcept;

  GGEMSTransportObserver &
  CapturePrimary(std::uint64_t global_primary_id) noexcept;
  GGEMSTransportObserver &ClearCapturedPrimary() noexcept;

  void Clear();
  void Accumulate(std::span<GGEMSObserverRecord const> records,
                  GGEMSObserverCounters const &counters);

  [[nodiscard]] GGEMSObserverConfigRecord BuildConfigRecord() const noexcept;

  [[nodiscard]] bool IsEnabled() const noexcept;
  [[nodiscard]] std::uint32_t GetRecordCapacity() const noexcept;
  [[nodiscard]] std::uint32_t GetRecordCount() const noexcept;
  [[nodiscard]] std::uint32_t GetOverflowCount() const noexcept;
  [[nodiscard]] std::uint32_t GetCapturedPrimaryCount() const noexcept;

  [[nodiscard]] std::vector<GGEMSObserverRecord> const &
  GetRecords() const noexcept;

  [[nodiscard]] std::string
  BuildDump(std::uint32_t max_record_count = 128U) const;
  void Verbose(std::uint32_t max_record_count = 128U) const;

private:
  bool enabled_{false};

  std::uint32_t record_capacity_{4096U};
  std::uint32_t max_stored_record_count_{65'536U};

  std::uint32_t capture_first_primary_count_{0U};
  bool capture_specific_primary_enabled_{false};
  std::uint64_t capture_global_primary_id_{0xFFFFFFFFFFFFFFFFULL};

  GGEMSObserverCounters counters_{};
  std::vector<GGEMSObserverRecord> records_{};
};

} // namespace ggems::core::observer
