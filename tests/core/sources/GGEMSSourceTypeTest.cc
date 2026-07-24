#include <array>
#include <cstdint>
#include <string_view>
#include <utility>

#include <gtest/gtest.h>

#include "GGEMS/core/sources/GGEMSSourceTypes.hh"

// =============================================================================
// =============================================================================

TEST(GGEMSSourceTypes, EmissionGeometryKernelIdsAreStable) {
  using enum ggems::core::sources::GGEMSEmissionGeometryType;
  using ggems::core::sources::FromKernelEmissionGeometryType;
  using ggems::core::sources::ToKernelEmissionGeometryType;

  constexpr std::array cases{std::pair{Unknown, 0U}, std::pair{Point, 1U},
                             std::pair{Rectangle, 2U}, std::pair{Ellipse, 3U}};

  for (auto const &[type, kernel_id] : cases) {
    EXPECT_EQ(ToKernelEmissionGeometryType(type), kernel_id);
    EXPECT_EQ(FromKernelEmissionGeometryType(kernel_id), type);
  }

  EXPECT_EQ(FromKernelEmissionGeometryType(99U), Unknown);
  EXPECT_EQ(ggems::core::sources::ToLongName(Point), "Point");
  EXPECT_EQ(ggems::core::sources::ToLongName(Rectangle), "Rectangle");
  EXPECT_EQ(ggems::core::sources::ToLongName(Ellipse), "Ellipse");
}

// =============================================================================
// =============================================================================

TEST(GGEMSSourceTypes, AngularDistributionKernelIdsAreStable) {
  using enum ggems::core::sources::GGEMSAngularDistributionType;
  using ggems::core::sources::FromKernelAngularDistributionType;
  using ggems::core::sources::ToKernelAngularDistributionType;

  constexpr std::array cases{std::pair{Unknown, 0U}, std::pair{Fixed, 1U},
                             std::pair{Isotropic, 2U}, std::pair{Focused, 3U}};

  for (auto const &[type, kernel_id] : cases) {
    EXPECT_EQ(ToKernelAngularDistributionType(type), kernel_id);
    EXPECT_EQ(FromKernelAngularDistributionType(kernel_id), type);
  }

  EXPECT_EQ(FromKernelAngularDistributionType(99U), Unknown);
  EXPECT_EQ(ggems::core::sources::ToLongName(Fixed), "Fixed");
  EXPECT_EQ(ggems::core::sources::ToLongName(Isotropic), "Isotropic");
  EXPECT_EQ(ggems::core::sources::ToLongName(Focused), "Focused");
}
