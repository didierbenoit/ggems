#include <memory>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/GGEMSRun.hh"
#include "GGEMS/core/random/GGEMSRandom.hh"
#include "GGEMS/frameworks/GGEMSOpenCL.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSRunTest, RejectsSecondInitialiseAndRemainsUsable) {
  auto &opencl = ggems::ocl::GGEMSOpenCL::GetInstance();

  if (opencl.GetContext().empty()) {
    opencl.SelectDevices({"gpu"});
    opencl.Initialise();
  }

  ASSERT_FALSE(opencl.GetContext().empty());

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
