#include <algorithm>
#include <cstdint>
#include <memory>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/observer/GGEMSTransportObserver.hh"
#include "GGEMS/core/particles/GGEMSParticleTypes.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/core/sources/GGEMSSource.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/core/observer/GGEMSObserverRecord.hh"
#include "GGEMS/core/sources/GGEMSSourceRecord.hh"

namespace {

constexpr std::uint32_t k_source_record_kind =
    ggems::core::observer::ToKernelObserverRecordKind(
        ggems::core::observer::GGEMSObserverRecordKind::Source);

// =============================================================================
// =============================================================================

bool IsSourceRecord(
    ggems::core::observer::GGEMSObserverRecord const &record) noexcept {
  return record.record_kind == k_source_record_kind;
}

// =============================================================================
// =============================================================================

void ExpectObserverSourceMatches(
    ggems::core::observer::GGEMSObserverRecord const &observed,
    ggems::core::sources::GGEMSSourceRecord const &expected) {
  EXPECT_EQ(observed.record_kind, k_source_record_kind);
  EXPECT_EQ(observed.particle_type, expected.emitted_particle_type);
  EXPECT_EQ(observed.time_ps, expected.time_start_ps);

  EXPECT_EQ(observed.position_x_pm, expected.position_x_pm);
  EXPECT_EQ(observed.position_y_pm, expected.position_y_pm);
  EXPECT_EQ(observed.position_z_pm, expected.position_z_pm);

  EXPECT_FLOAT_EQ(observed.direction_x, expected.direction_x);
  EXPECT_FLOAT_EQ(observed.direction_y, expected.direction_y);
  EXPECT_FLOAT_EQ(observed.direction_z, expected.direction_z);
  EXPECT_FLOAT_EQ(observed.direction_w, expected.direction_w);

  EXPECT_EQ(observed.energy_milli_eV, expected.energy_milli_eV);
  EXPECT_FLOAT_EQ(observed.weight, expected.weight);
}

// =============================================================================
// =============================================================================

class GGEMSRunTest : public ::testing::Test {
protected:
  static void SetUpTestSuite() {
    auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

    if (opencl.GetContext().empty()) {
      opencl.SelectDevices({"gpu"});
      opencl.Initialise();
    }

    ASSERT_FALSE(opencl.GetContext().empty());
  }
};
} // namespace

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, RejectsSecondInitialiseAndRemainsUsable) {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7777777ULL);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.SetPrimaryCount(1U);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());

  EXPECT_THROW(run.Initialise(), ggems::core::GGEMSExceptionBase);

  EXPECT_NO_THROW(run.Run());
  EXPECT_NO_THROW(run.Run());
}

// =============================================================================
// =============================================================================

TEST_F(GGEMSRunTest, UsesIndependentSourceSnapshotsAcrossSequentialRuns) {
  auto random = std::make_shared<ggems::core::random::GGEMSRandom>();
  random->SetEngine("philox");
  random->SetSeed(7777777ULL);

  auto source = std::make_shared<ggems::core::sources::GGEMSSource>();

  source->SetAnalytic()
      .SetEmittedParticleType(ggems::core::particles::GGEMSParticleType::Gamma)
      .SetEnergyMilliElectronVolt(1'000'000ULL)
      .SetTimeWindowPicoSecond(100ULL, 100ULL)
      .SetPositionPicoMeter(10LL, -20LL, 30LL)
      .SetDirection(1.0F, 0.0F, 0.0F)
      .SetWeight(0.25F);

  auto expected_a = source->BuildRecord();

  ASSERT_EQ(expected_a.time_start_ps, expected_a.time_stop_ps);

  auto observer =
      std::make_shared<ggems::core::observer::GGEMSTransportObserver>();
  observer->CaptureFirstPrimaries(1U);

  ggems::core::GGEMSRun run{};
  run.SetRandom(random);
  run.SetSource(source);
  run.SetObserver(observer);
  run.SetPrimaryCount(1U);
  run.SetWorkerCount(64U);

  ASSERT_NO_THROW(run.Initialise());
  ASSERT_NO_THROW(run.Run());

  auto &records_after_first_run = observer->GetRecords();

  auto first_source =
      std::find_if(records_after_first_run.begin(),
                   records_after_first_run.end(), IsSourceRecord);

  ASSERT_NE(first_source, records_after_first_run.end());
  ASSERT_EQ(std::count_if(records_after_first_run.begin(),
                          records_after_first_run.end(), IsSourceRecord),
            1);

  auto first_source_record = *first_source;
  ExpectObserverSourceMatches(first_source_record, expected_a);

  source
      ->SetEmittedParticleType(
          ggems::core::particles::GGEMSParticleType::Electron)
      .SetEnergyMilliElectronVolt(2'000'000ULL)
      .SetTimeWindowPicoSecond(200ULL, 200ULL)
      .SetPositionPicoMeter(-40LL, 50LL, -60LL)
      .SetDirection(0.0F, -1.0F, 0.0F)
      .SetWeight(0.75F);

  auto expected_b = source->BuildRecord();

  ASSERT_EQ(expected_b.time_start_ps, expected_b.time_stop_ps);

  ASSERT_NO_THROW(run.Run());

  auto &all_records = observer->GetRecords();

  ASSERT_EQ(
      std::count_if(all_records.begin(), all_records.end(), IsSourceRecord), 2);

  auto retained_first_source = std::find_if(
      all_records.begin(), all_records.end(),
      [first_run_id = first_source_record.run_id](
          ggems::core::observer::GGEMSObserverRecord const &record) {
        return IsSourceRecord(record) && record.run_id == first_run_id;
      });

  auto second_source = std::find_if(
      all_records.begin(), all_records.end(),
      [first_run_id = first_source_record.run_id](
          ggems::core::observer::GGEMSObserverRecord const &record) {
        return IsSourceRecord(record) && record.run_id != first_run_id;
      });

  ASSERT_NE(retained_first_source, all_records.end());
  ASSERT_NE(second_source, all_records.end());

  EXPECT_EQ(retained_first_source->global_primary_id,
            first_source_record.global_primary_id);
  EXPECT_NE(second_source->run_id, first_source_record.run_id);

  ExpectObserverSourceMatches(*retained_first_source, expected_a);
  ExpectObserverSourceMatches(*second_source, expected_b);
}
