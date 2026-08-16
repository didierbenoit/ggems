#include <array>
#include <cmath>
#include <cstdint>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/units/GGEMSDoseUnits.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"
#include "GGEMS/core/units/GGEMSUnitFormatting.hh"
#include "GGEMSScopedLoggerEncoding.hh"

namespace {

using ggems::test::ScopedLoggerEncoding;
using ggems::units::ConvertTo;
using ggems::units::Dose;
using ggems::units::HumanReadable;
using ggems::units::MakeQuantity;
using ggems::units::UnitConversionError;

constexpr long double k_gray_per_canonical_unit = 1.602176634e-7L;

TEST(GGEMSDoseUnitsTest, ConvertsEveryOfficialRuntimeToken) {
  struct Case {
    std::string_view unit;
    std::uint64_t expected_milli_electron_volts_per_picogram;
  };

  constexpr std::array<Case, 4U> cases{{
      {.unit = "meV/pg", .expected_milli_electron_volts_per_picogram = 1ULL},
      {.unit = "Gy",
       .expected_milli_electron_volts_per_picogram = 6'241'509ULL},
      {.unit = "mGy", .expected_milli_electron_volts_per_picogram = 6'242ULL},
      {.unit = "uGy", .expected_milli_electron_volts_per_picogram = 6ULL},
  }};

  for (auto const &test_case : cases) {
    SCOPED_TRACE(test_case.unit);
    auto const converted = MakeQuantity<Dose>(1.0L, test_case.unit);

    ASSERT_TRUE(converted.has_value());
    EXPECT_EQ(converted->value,
              test_case.expected_milli_electron_volts_per_picogram);
  }
}

TEST(GGEMSDoseUnitsTest, ConvertsIntegerCanonicalDoseBackToGray) {
  auto const gray = ConvertTo(Dose{6'241'509ULL}, "Gy");

  ASSERT_TRUE(gray.has_value());
  EXPECT_LE(std::abs(*gray - 1.0L), 0.5L * k_gray_per_canonical_unit);
}

TEST(GGEMSDoseUnitsTest, RejectsNegativeDose) {
  auto const converted = MakeQuantity<Dose>(-1.0L, "Gy");

  ASSERT_FALSE(converted.has_value());
  EXPECT_EQ(converted.error(), UnitConversionError::NegativeValue);
}

TEST(GGEMSDoseUnitsTest, PresentsMicrograyInAsciiAndUnicode) {
  Dose const quantum{1ULL};

  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Ascii};
    EXPECT_EQ(HumanReadable(quantum, 3), "0.160 uGy");
  }
  {
    ScopedLoggerEncoding const encoding{ggems::core::Encoding::Unicode};
    EXPECT_EQ(HumanReadable(quantum, 3), "0.160 µGy");
  }
}

TEST(GGEMSDoseUnitsTest, GrayLiteralUsesCanonicalIntegerQuantization) {
  using namespace ggems::units;

  constexpr Dose gray = 1_Gy;

  EXPECT_EQ(gray.value, 6'241'509ULL);
}

} // namespace
