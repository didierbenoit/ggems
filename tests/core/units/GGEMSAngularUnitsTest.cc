#include <cmath>
#include <format>
#include <string>

#include <gtest/gtest.h>

#include "GGEMS/core/units/GGEMSAngularUnits.hh"

namespace {

constexpr long double k_tolerance{1.0e-12L};

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

void ExpectNearLongDouble(long double value, long double reference,
                          long double tolerance = k_tolerance) {
  EXPECT_LE(std::abs(value - reference), tolerance);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, DefaultAngleIsZeroRadians) {
  ggems::units::Angle angle{};
  ExpectNearLongDouble(ggems::units::ToRadians(angle), 0.0L);
  ExpectNearLongDouble(ggems::units::ToDegrees(angle), 0.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, MakeRadiansStoresRadiansDirectly) {
  ggems::units::Angle angle = ggems::units::MakeRadians(2.5L);

  ExpectNearLongDouble(ggems::units::ToRadians(angle), 2.5L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, MakeDegreesConvertsToRadians) {
  ggems::units::Angle angle = ggems::units::MakeDegrees(180.0L);

  ExpectNearLongDouble(ggems::units::ToRadians(angle),
                       ggems::units::detail::k_pi);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, RadiansAreConvertedToDegrees) {
  ggems::units::Angle const angle =
      ggems::units::MakeRadians(ggems::units::detail::k_pi);

  ExpectNearLongDouble(ggems::units::ToDegrees(angle), 180.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, DegreeLiteralStoresRadiansInternally) {
  using namespace ggems::units;

  Angle angle = 90.0_deg;

  ExpectNearLongDouble(ToRadians(angle), detail::k_pi / 2.0L);
  ExpectNearLongDouble(ToDegrees(angle), 90.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, IntegerDegreeLiteralStoresRadiansInternally) {
  using namespace ggems::units;

  Angle angle = 270_deg;

  ExpectNearLongDouble(ToRadians(angle), 3.0L * detail::k_pi / 2.0L);
  ExpectNearLongDouble(ToDegrees(angle), 270.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, RadianLiteralStoresRadiansDirectly) {
  using namespace ggems::units;

  Angle angle = 2.0_rad;

  ExpectNearLongDouble(ToRadians(angle), 2.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, IntegerRadianLiteralStoresRadiansDirectly) {
  using namespace ggems::units;

  Angle const angle = 2_rad;

  ExpectNearLongDouble(ToRadians(angle), 2.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, SupportsNegativeAngles) {
  using namespace ggems::units;

  Angle angle = -45.0_deg;

  ExpectNearLongDouble(ToDegrees(angle), -45.0L);
  ExpectNearLongDouble(ToRadians(angle), -detail::k_pi / 4.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, AdditionPreservesAngleSemantics) {
  using namespace ggems::units;

  Angle lhs = 30.0_deg;
  Angle rhs = 15.0_deg;

  ExpectNearLongDouble(ToDegrees(lhs + rhs), 45.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, SubtractionPreservesAngleSemantics) {
  using namespace ggems::units;

  Angle lhs = 30.0_deg;
  Angle rhs = 15.0_deg;

  ExpectNearLongDouble(ToDegrees(lhs - rhs), 15.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, UnaryMinusPreservesAngleSemantics) {
  using namespace ggems::units;

  Angle angle = 30.0_deg;

  ExpectNearLongDouble(ToDegrees(-angle), -30.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, MultiplicationByScalarPreservesAngleSemantics) {
  using namespace ggems::units;

  Angle angle = 30.0_deg;

  ExpectNearLongDouble(ToDegrees(angle * 2.0L), 60.0L);
  ExpectNearLongDouble(ToDegrees(2.0L * angle), 60.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, DivisionByScalarPreservesAngleSemantics) {
  using namespace ggems::units;

  Angle angle = 30.0_deg;

  ExpectNearLongDouble(ToDegrees(angle / 2.0L), 15.0L);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, SpaceshipComparisonUsesRadians) {
  using namespace ggems::units;

  EXPECT_EQ(90.0_deg, MakeRadians(detail::k_pi / 2.0L));
  EXPECT_LT(45.0_deg, 90.0_deg);
  EXPECT_GT(180.0_deg, 90.0_deg);
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, HumanReadableUsesDegreesByDefault) {
  using namespace ggems::units;

  Angle angle = 45.0_deg;

  EXPECT_EQ(HumanReadable(angle), "45.000 deg");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, HumanReadableSupportsPrecision) {
  using namespace ggems::units;

  Angle angle = 12.3456_deg;

  EXPECT_EQ(HumanReadable(angle, 0), "12 deg");
  EXPECT_EQ(HumanReadable(angle, 1), "12.3 deg");
  EXPECT_EQ(HumanReadable(angle, 2), "12.35 deg");
  EXPECT_EQ(HumanReadable(angle, 3), "12.346 deg");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, HumanReadableSupportsWidth) {
  using namespace ggems::units;

  Angle angle = 12.5_deg;

  EXPECT_EQ(HumanReadable(angle, 1, 6), "  12.5 deg");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, StdFormatterUsesDefaultHumanReadableDegrees) {
  using namespace ggems::units;

  Angle angle = 12.3456_deg;

  EXPECT_EQ(std::format("{}", angle), "12.346 deg");
}

/* --------------------------------------------- */
/* --------------------------------------------- */
/* --------------------------------------------- */

TEST(GGEMSAngularUnits, StdFormatterSupportsSingleDigitPrecision) {
  using namespace ggems::units;

  Angle angle = 12.3456_deg;

  EXPECT_EQ(std::format("{:0}", angle), "12 deg");
  EXPECT_EQ(std::format("{:1}", angle), "12.3 deg");
  EXPECT_EQ(std::format("{:2}", angle), "12.35 deg");
}

} // namespace
