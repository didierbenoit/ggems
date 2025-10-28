// ************************************************************************
// * This file is part of GGEMS.                                          *
// *                                                                      *
// * GGEMS is free software: you can redistribute it and/or modify        *
// * it under the terms of the GNU General Public License as published by *
// * the Free Software Foundation, either version 3 of the License, or    *
// * (at your option) any later version.                                  *
// *                                                                      *
// * GGEMS is distributed in the hope that it will be useful,             *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of       *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
// * GNU General Public License for more details.                         *
// *                                                                      *
// * You should have received a copy of the GNU General Public License    *
// * along with GGEMS.  If not, see <https://www.gnu.org/licenses/>.      *
// *                                                                      *
// ************************************************************************

#include <gtest/gtest.h>
#include <memory>

#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"

using GGEMSOpenCLPlaftormPtr = std::unique_ptr<GGEMSOpenCLPlatform>;

class GGEMSOpenCLPlatformTest : public ::testing::Test {
protected:
  GGEMSOpenCLPlatformTest(void) {
    std::vector<cl::Platform> platforms;
    cl::Platform::get(&platforms);

    // Testing first platform only
    platform_ = std::make_unique<GGEMSOpenCLPlatform>(platforms.front(), 0);
  }

  ~GGEMSOpenCLPlatformTest(void) {
    platform_->Clean();
  }

  void SetUp(void) override {}

  void TearDown(void) override {}

protected:
  GGEMSOpenCLPlaftormPtr platform_;
};

TEST_F(GGEMSOpenCLPlatformTest, AtLeastOnePlatformAvailable) {
  ASSERT_TRUE(platform_);
}

TEST_F(GGEMSOpenCLPlatformTest, GetPlatformName) {
  EXPECT_FALSE(platform_->GetName().empty());
}

TEST_F(GGEMSOpenCLPlatformTest, GetVersion) {
  EXPECT_FALSE(platform_->GetVersion().empty());
}

TEST_F(GGEMSOpenCLPlatformTest, CheckAvailableExtension) {
  EXPECT_TRUE(platform_->CheckExtension("cl_khr_global_int32_base_atomics"));
}

TEST_F(GGEMSOpenCLPlatformTest, GetVendorName) {
  EXPECT_FALSE(platform_->GetVendor().empty());
}
