#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"

namespace ggems::core {
class GGEMSRun;
}

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

  auto CaptureFirstPrimaries(std::uint32_t primary_count_per_source) noexcept
      -> GGEMSTransportObserver &;

  auto CapturePrimary(std::uint32_t source_index,
                      std::uint64_t source_local_primary_id) noexcept
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

  [[nodiscard]] auto BuildDump() const -> std::string;

private:
  friend class ggems::core::GGEMSRun;

  explicit GGEMSTransportObserver(bool reserve_record_capacity);

  auto SwapRunResult(GGEMSTransportObserver &other) noexcept -> void;

  bool enabled_{false};

  std::uint32_t record_capacity_{65'536U};
  std::uint32_t max_stored_record_count_{262'144U};

  std::uint32_t capture_first_primary_count_per_source_{0U};
  bool capture_specific_primary_enabled_{false};
  std::uint32_t capture_source_index_{particles::k_invalid_id_u32};
  std::uint64_t capture_source_local_primary_id_{particles::k_invalid_id_u64};

  GGEMSObserverCounters counters_{};
  std::vector<GGEMSObserverRecord> records_;
};

} // namespace ggems::core::observer
