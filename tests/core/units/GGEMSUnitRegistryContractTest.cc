#include <array>
#include <cstdint>
#include <format>
#include <limits>
#include <string>
#include <type_traits>
#include <string_view>

#include <gtest/gtest.h>

#include "GGEMS/core/units/GGEMSActivityUnits.hh"
#include "GGEMS/core/units/GGEMSAngularUnits.hh"
#include "GGEMS/core/units/GGEMSAreaUnits.hh"
#include "GGEMS/core/units/GGEMSBitsUnits.hh"
#include "GGEMS/core/units/GGEMSBytesUnits.hh"
#include "GGEMS/core/units/GGEMSCrossSectionUnits.hh"
#include "GGEMS/core/units/GGEMSDensityUnits.hh"
#include "GGEMS/core/units/GGEMSDoseUnits.hh"
#include "GGEMS/core/units/GGEMSEnergyUnits.hh"
#include "GGEMS/core/units/GGEMSFrequencyUnits.hh"
#include "GGEMS/core/units/GGEMSLengthUnits.hh"
#include "GGEMS/core/units/GGEMSMassUnits.hh"
#include "GGEMS/core/units/GGEMSSpeedUnits.hh"
#include "GGEMS/core/units/GGEMSTimeUnits.hh"
#include "GGEMS/core/units/GGEMSVolumeUnits.hh"
#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {

// =============================================================================
// =============================================================================

struct MissingCanonicalUnitSet {
  using dimension = LengthDim;
};

template <> struct UnitRegistry<MissingCanonicalUnitSet> {
  static constexpr std::array<UnitDefinition, 1> units{{
      {.canonical_name = "test",
       .symbol = "test",
       .display_symbol = "test",
       .literal_suffix = "test",
       .scale = DecimalScale(0)},
  }};
};

struct DuplicateTokenUnitSet {
  using dimension = LengthDim;
};

template <> struct UnitRegistry<DuplicateTokenUnitSet> {
  static constexpr std::array<UnitDefinition, 2> units{{
      {.canonical_name = "first",
       .symbol = "first",
       .display_symbol = "first",
       .aliases = {"shared"},
       .literal_suffix = "first",
       .scale = DecimalScale(0),
       .canonical = true,
       .automatic_display = true},
      {.canonical_name = "second",
       .symbol = "second",
       .display_symbol = "second",
       .aliases = {"shared"},
       .literal_suffix = "second",
       .scale = DecimalScale(1),
       .automatic_display = true},
  }};
};

struct DuplicateLiteralSuffixUnitSet {
  using dimension = LengthDim;
};

template <> struct UnitRegistry<DuplicateLiteralSuffixUnitSet> {
  static constexpr std::array<UnitDefinition, 2> units{{
      {.canonical_name = "first",
       .symbol = "first",
       .display_symbol = "first",
       .literal_suffix = "duplicate",
       .scale = DecimalScale(0),
       .canonical = true,
       .automatic_display = true},
      {.canonical_name = "second",
       .symbol = "second",
       .display_symbol = "second",
       .literal_suffix = "duplicate",
       .scale = DecimalScale(1),
       .automatic_display = true},
  }};
};

} // namespace ggems::units

namespace {

// =============================================================================
// =============================================================================

using namespace ggems::units;

template <typename Left, typename Right>
concept Addable = requires(Left left, Right right) { left + right; };

template <typename Left, typename Right>
concept Comparable = requires(Left left, Right right) { left <=> right; };

struct SignedUnsignedFamily {
  using dimension = LengthDim;
  using unit_set = LengthUnitSet;
  using representation = std::uint64_t;
  static constexpr std::string_view name{"SignedUnsigned"};
  static constexpr QuantityDomain domain{QuantityDomain::Signed};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::AutomaticScale};
  static constexpr std::string_view fixed_display_unit{};
  static constexpr std::int8_t default_precision{7};
};

struct MissingFixedUnitFamily {
  using dimension = LengthDim;
  using unit_set = LengthUnitSet;
  using representation = std::uint64_t;
  static constexpr std::string_view name{"MissingFixedUnit"};
  static constexpr QuantityDomain domain{QuantityDomain::NonNegative};
  static constexpr QuantityFormatPolicy format_policy{
      QuantityFormatPolicy::FixedUnit};
  static constexpr std::string_view fixed_display_unit{"absent"};
  static constexpr std::int8_t default_precision{7};
};

// =============================================================================
// =============================================================================

static_assert(ValidateUnitSet<ActivityUnitSet>());
static_assert(ValidateUnitSet<AngleUnitSet>());
static_assert(ValidateUnitSet<AreaUnitSet>());
static_assert(ValidateUnitSet<BitsUnitSet>());
static_assert(ValidateUnitSet<BytesUnitSet>());
static_assert(ValidateUnitSet<CrossSectionUnitSet>());
static_assert(ValidateUnitSet<DensityUnitSet>());
static_assert(ValidateUnitSet<DoseUnitSet>());
static_assert(ValidateUnitSet<EnergyUnitSet>());
static_assert(ValidateUnitSet<FrequencyUnitSet>());
static_assert(ValidateUnitSet<LengthUnitSet>());
static_assert(ValidateUnitSet<MassUnitSet>());
static_assert(ValidateUnitSet<SpeedUnitSet>());
static_assert(ValidateUnitSet<TimeUnitSet>());
static_assert(ValidateUnitSet<VolumeUnitSet>());

static_assert(ValidateFamily<ActivityFamily>());
static_assert(ValidateFamily<AngleFamily>());
static_assert(ValidateFamily<AreaFamily>());
static_assert(ValidateFamily<BitsFamily>());
static_assert(ValidateFamily<BytesFamily>());
static_assert(ValidateFamily<CrossSectionFamily>());
static_assert(ValidateFamily<DensityFamily>());
static_assert(ValidateFamily<DoseFamily>());
static_assert(ValidateFamily<EnergyFamily>());
static_assert(ValidateFamily<EnergyChangeFamily>());
static_assert(ValidateFamily<FrequencyFamily>());
static_assert(ValidateFamily<LengthFamily>());
static_assert(ValidateFamily<PositionCoordinateFamily>());
static_assert(ValidateFamily<DisplacementFamily>());
static_assert(ValidateFamily<MassFamily>());
static_assert(ValidateFamily<SpeedFamily>());
static_assert(ValidateFamily<DurationFamily>());
static_assert(ValidateFamily<TimePointFamily>());
static_assert(ValidateFamily<VolumeFamily>());

static_assert(!ValidateUnitSet<MissingCanonicalUnitSet>());
static_assert(!ValidateUnitSet<DuplicateTokenUnitSet>());
static_assert(!ValidateUnitSet<DuplicateLiteralSuffixUnitSet>());
static_assert(!ValidateFamily<SignedUnsignedFamily>());
static_assert(!ValidateFamily<MissingFixedUnitFamily>());

// =============================================================================
// =============================================================================

static_assert(std::is_same_v<Activity::dimension, Frequency::dimension>);
static_assert(!std::is_same_v<Activity, Frequency>);
static_assert(std::is_same_v<Area::dimension, CrossSection::dimension>);
static_assert(!std::is_same_v<Area, CrossSection>);
static_assert(std::is_same_v<Bits::dimension, Bytes::dimension>);
static_assert(!std::is_same_v<Bits, Bytes>);
static_assert(std::is_same_v<Length::dimension, PositionCoordinate::dimension>);
static_assert(!std::is_same_v<Length, PositionCoordinate>);
static_assert(!std::is_same_v<PositionCoordinate, Displacement>);
static_assert(std::is_same_v<Energy::dimension, EnergyChange::dimension>);
static_assert(!std::is_same_v<Energy, EnergyChange>);
static_assert(std::is_same_v<Duration::dimension, TimePoint::dimension>);
static_assert(!std::is_same_v<Duration, TimePoint>);
static_assert(std::is_same_v<Time, Duration>);

static_assert(Addable<Length, Length>);
static_assert(Comparable<Length, Length>);
static_assert(!Addable<Length, PositionCoordinate>);
static_assert(!Comparable<Length, PositionCoordinate>);
static_assert(!Addable<Area, CrossSection>);
static_assert(!Comparable<Bits, Bytes>);
static_assert(!std::is_convertible_v<Area, CrossSection>);
static_assert(!std::is_convertible_v<Bits, Bytes>);

// =============================================================================
// =============================================================================

consteval auto EveryRegisteredLiteralCompiles() -> bool {
  auto const activity = std::array{
      1_Bq,   1_kBq,   1_MBq,   1_GBq,   1_TBq,   1_Ci,   1_mCi,   1_uCi,
      1.0_Bq, 1.0_kBq, 1.0_MBq, 1.0_GBq, 1.0_TBq, 1.0_Ci, 1.0_mCi, 1.0_uCi};
  auto const angle = std::array{1_rad, 1_deg, 1.0_rad, 1.0_deg};
  auto const area =
      std::array{1_pm2,   1_nm2,   1_um2,   1_mm2,   1_cm2,   1_m2,   1_km2,
                 1.0_pm2, 1.0_nm2, 1.0_um2, 1.0_mm2, 1.0_cm2, 1.0_m2, 1.0_km2};
  auto const bits = std::array{
      1_bit,    1_kbit,   1_Mbit,    1_Gbit,    1_Tbit,    1_Kibit,
      1_Mibit,  1_Gibit,  1_Tibit,   1.0_bit,   1.0_kbit,  1.0_Mbit,
      1.0_Gbit, 1.0_Tbit, 1.0_Kibit, 1.0_Mibit, 1.0_Gibit, 1.0_Tibit};
  auto const bytes =
      std::array{1_B,    1_kB,   1_MB,    1_GB,    1_TB,    1_KiB,
                 1_MiB,  1_GiB,  1_TiB,   1.0_B,   1.0_kB,  1.0_MB,
                 1.0_GB, 1.0_TB, 1.0_KiB, 1.0_MiB, 1.0_GiB, 1.0_TiB};
  auto const cross_section =
      std::array{1_pb,   1_nb,   1_ub,   1_mb,   1_barn,   1_kbarn,
                 1.0_pb, 1.0_nb, 1.0_ub, 1.0_mb, 1.0_barn, 1.0_kbarn};
  auto const density = std::array{1_pg_pm3, 1_g_cm3, 1.0_pg_pm3, 1.0_g_cm3};
  auto const dose = std::array{1_meV_pg,   1_Gy,   1_mGy,   1_uGy,
                               1.0_meV_pg, 1.0_Gy, 1.0_mGy, 1.0_uGy};
  auto const energy =
      std::array{1_meV,   1_eV,   1_keV,   1_MeV,   1_GeV,   1_TeV,
                 1.0_meV, 1.0_eV, 1.0_keV, 1.0_MeV, 1.0_GeV, 1.0_TeV};
  auto const frequency = std::array{1_Hz,   1_kHz,   1_MHz,   1_GHz,   1_THz,
                                    1.0_Hz, 1.0_kHz, 1.0_MHz, 1.0_GHz, 1.0_THz};
  auto const length =
      std::array{1_pm,   1_nm,   1_um,   1_mm,   1_cm,   1_m,   1_km,
                 1.0_pm, 1.0_nm, 1.0_um, 1.0_mm, 1.0_cm, 1.0_m, 1.0_km};
  auto const mass = std::array{1_pg,   1_ng,   1_ug,   1_mg,   1_g,   1_kg,
                               1.0_pg, 1.0_ng, 1.0_ug, 1.0_mg, 1.0_g, 1.0_kg};
  auto const speed = std::array{1_pm_ps, 1_m_s, 1.0_pm_ps, 1.0_m_s};
  auto const time =
      std::array{1_ps,   1_ns,   1_us,   1_ms,   1_s,   1_min,   1_h,
                 1.0_ps, 1.0_ns, 1.0_us, 1.0_ms, 1.0_s, 1.0_min, 1.0_h};
  auto const volume =
      std::array{1_pm3,   1_nm3,   1_um3,   1_mm3,   1_cm3,   1_m3,   1_km3,
                 1.0_pm3, 1.0_nm3, 1.0_um3, 1.0_mm3, 1.0_cm3, 1.0_m3, 1.0_km3};
  static_cast<void>(activity);
  static_cast<void>(angle);
  static_cast<void>(area);
  static_cast<void>(bits);
  static_cast<void>(bytes);
  static_cast<void>(cross_section);
  static_cast<void>(density);
  static_cast<void>(dose);
  static_cast<void>(energy);
  static_cast<void>(frequency);
  static_cast<void>(length);
  static_cast<void>(mass);
  static_cast<void>(speed);
  static_cast<void>(time);
  static_cast<void>(volume);
  return true;
}

static_assert(EveryRegisteredLiteralCompiles());
static_assert((1_cm).value == 10'000'000'000ULL);
static_assert((1_cm2).value == 100'000'000'000'000'000'000.0L);
static_assert((1_km).value == 1'000'000'000'000'000ULL);
static_assert((1_pb).value == 1ULL);
static_assert((1_barn).value == 1'000'000'000'000ULL);
static_assert((1_KiB).value == 1'024ULL);
static_assert((1_kB).value == 1'000ULL);
static_assert((1_Kibit).value == 1'024ULL);
static_assert((1_kbit).value == 1'000ULL);
static_assert((1_Gy).value == 6'241'509ULL);

// =============================================================================
// =============================================================================

TEST(GGEMSUnitRegistryContractTest, ParsesOnlyExplicitCaseSensitiveTokens) {
  auto const micrometer_ascii = TryMakeQuantity<Length>(1.0L, "um");
  auto const micrometer_micro_sign = TryMakeQuantity<Length>(1.0L, "µm");
  auto const micrometer_greek_mu = TryMakeQuantity<Length>(1.0L, "μm");
  ASSERT_TRUE(micrometer_ascii.has_value());
  ASSERT_TRUE(micrometer_micro_sign.has_value());
  ASSERT_TRUE(micrometer_greek_mu.has_value());
  EXPECT_EQ(micrometer_ascii->value, 1'000'000ULL);
  EXPECT_EQ(*micrometer_micro_sign, *micrometer_ascii);
  EXPECT_EQ(*micrometer_greek_mu, *micrometer_ascii);

  auto const square_micrometer_ascii = TryMakeQuantity<Area>(1.0L, "um2");
  auto const square_micrometer_micro = TryMakeQuantity<Area>(1.0L, "µm2");
  auto const square_micrometer_greek = TryMakeQuantity<Area>(1.0L, "μm²");
  ASSERT_TRUE(square_micrometer_ascii.has_value());
  ASSERT_TRUE(square_micrometer_micro.has_value());
  ASSERT_TRUE(square_micrometer_greek.has_value());
  EXPECT_EQ(*square_micrometer_micro, *square_micrometer_ascii);
  EXPECT_EQ(*square_micrometer_greek, *square_micrometer_ascii);

  auto const microcurie_ascii = TryMakeQuantity<Activity>(1.0L, "uCi");
  auto const microcurie_micro_sign = TryMakeQuantity<Activity>(1.0L, "µCi");
  auto const microcurie_greek_mu = TryMakeQuantity<Activity>(1.0L, "μCi");
  ASSERT_TRUE(microcurie_ascii.has_value());
  ASSERT_TRUE(microcurie_micro_sign.has_value());
  ASSERT_TRUE(microcurie_greek_mu.has_value());
  EXPECT_EQ(microcurie_ascii->value, 37'000.0L);
  EXPECT_EQ(*microcurie_micro_sign, *microcurie_ascii);
  EXPECT_EQ(*microcurie_greek_mu, *microcurie_ascii);

  auto const upper_case = TryMakeQuantity<Length>(1.0L, "UM");
  auto const leading_space = TryMakeQuantity<Length>(1.0L, " um");
  ASSERT_FALSE(upper_case.has_value());
  ASSERT_FALSE(leading_space.has_value());
  EXPECT_EQ(upper_case.error(), UnitConversionError::UnsupportedUnit);
  EXPECT_EQ(leading_space.error(), UnitConversionError::UnsupportedUnit);
}

// =============================================================================
// =============================================================================

TEST(GGEMSUnitRegistryContractTest, ConvertsThroughCanonicalRepresentations) {
  auto const centimeter = TryMakeQuantity<Length>(1, "cm");
  auto const square_centimeter = TryMakeQuantity<Area>(1.0L, "cm2");
  auto const cubic_centimeter = TryMakeQuantity<Volume>(1.0L, "cm3");
  auto const barn = TryMakeQuantity<CrossSection>(1, "barn");
  auto const picobarn_alias = TryMakeQuantity<CrossSection>(1, "pbarn");
  auto const energy_alias = TryMakeQuantity<Energy>(1, "milli_eV");

  ASSERT_TRUE(centimeter.has_value());
  ASSERT_TRUE(square_centimeter.has_value());
  ASSERT_TRUE(cubic_centimeter.has_value());
  ASSERT_TRUE(barn.has_value());
  ASSERT_TRUE(picobarn_alias.has_value());
  ASSERT_TRUE(energy_alias.has_value());
  EXPECT_EQ(centimeter->value, 10'000'000'000ULL);
  EXPECT_EQ(square_centimeter->value, 1.0e20L);
  EXPECT_NEAR(static_cast<double>(cubic_centimeter->value), 1.0e30, 1.0e15);
  EXPECT_EQ(barn->value, 1'000'000'000'000ULL);
  EXPECT_EQ(picobarn_alias->value, 1ULL);
  EXPECT_EQ(energy_alias->value, 1ULL);

  auto const centimeter_round_trip = TryConvertTo(*centimeter, "cm");
  auto const barn_in_picobarns = TryConvertTo<std::uint64_t>(*barn, "pb");
  ASSERT_TRUE(centimeter_round_trip.has_value());
  ASSERT_TRUE(barn_in_picobarns.has_value());
  EXPECT_EQ(*centimeter_round_trip, 1.0L);
  EXPECT_EQ(*barn_in_picobarns, 1'000'000'000'000ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSUnitRegistryContractTest, AppliesTheNumericBoundaryContract) {
  auto const positive_below_half = TryMakeQuantity<Length>(0.49L, "pm");
  auto const positive_half = TryMakeQuantity<Length>(0.5L, "pm");
  auto const positive_above_half = TryMakeQuantity<Length>(1.5L, "pm");
  auto const negative_half = TryMakeQuantity<PositionCoordinate>(-0.5L, "pm");
  auto const negative_above_half =
      TryMakeQuantity<PositionCoordinate>(-1.5L, "pm");
  ASSERT_TRUE(positive_below_half.has_value());
  ASSERT_TRUE(positive_half.has_value());
  ASSERT_TRUE(positive_above_half.has_value());
  ASSERT_TRUE(negative_half.has_value());
  ASSERT_TRUE(negative_above_half.has_value());
  EXPECT_EQ(positive_below_half->value, 0ULL);
  EXPECT_EQ(positive_half->value, 1ULL);
  EXPECT_EQ(positive_above_half->value, 2ULL);
  EXPECT_EQ(negative_half->value, -1LL);
  EXPECT_EQ(negative_above_half->value, -2LL);

  auto const maximum_length =
      TryMakeQuantity<Length>(std::numeric_limits<std::uint64_t>::max(), "pm");
  auto const minimum_position = TryMakeQuantity<PositionCoordinate>(
      std::numeric_limits<std::int64_t>::min(), "pm");
  ASSERT_TRUE(maximum_length.has_value());
  ASSERT_TRUE(minimum_position.has_value());
  EXPECT_EQ(maximum_length->value, std::numeric_limits<std::uint64_t>::max());
  EXPECT_EQ(minimum_position->value, std::numeric_limits<std::int64_t>::min());

  auto const exact_maximum = TryConvertTo<std::uint64_t>(*maximum_length, "pm");
  auto const exact_minimum =
      TryConvertTo<std::int64_t>(*minimum_position, "pm");
  ASSERT_TRUE(exact_maximum.has_value());
  ASSERT_TRUE(exact_minimum.has_value());
  EXPECT_EQ(*exact_maximum, std::numeric_limits<std::uint64_t>::max());
  EXPECT_EQ(*exact_minimum, std::numeric_limits<std::int64_t>::min());

  auto const zero_dose_in_gray = TryConvertTo<std::uint64_t>(Dose{0ULL}, "Gy");
  auto const zero_energy = TryMakeQuantity<Energy>(0.0L, "keV");
  ASSERT_TRUE(zero_dose_in_gray.has_value());
  ASSERT_TRUE(zero_energy.has_value());
  EXPECT_EQ(*zero_dose_in_gray, 0ULL);
  EXPECT_EQ(zero_energy->value, 0ULL);
}

// =============================================================================
// =============================================================================

TEST(GGEMSUnitRegistryContractTest, ReportsEveryConversionErrorCategory) {
  auto const unsupported = TryMakeQuantity<Length>(1.0L, "parsec");
  auto const non_finite = TryMakeQuantity<Length>(
      std::numeric_limits<long double>::infinity(), "pm");
  auto const negative = TryMakeQuantity<Length>(-1, "pm");
  auto const overflow =
      TryMakeQuantity<Length>(std::numeric_limits<std::uint64_t>::max(), "nm");
  auto const inexact = TryConvertTo<std::uint64_t>(Length{1ULL}, "nm");

  ASSERT_FALSE(unsupported.has_value());
  ASSERT_FALSE(non_finite.has_value());
  ASSERT_FALSE(negative.has_value());
  ASSERT_FALSE(overflow.has_value());
  ASSERT_FALSE(inexact.has_value());
  EXPECT_EQ(unsupported.error(), UnitConversionError::UnsupportedUnit);
  EXPECT_EQ(non_finite.error(), UnitConversionError::NonFinite);
  EXPECT_EQ(negative.error(), UnitConversionError::NegativeValue);
  EXPECT_EQ(overflow.error(), UnitConversionError::OutOfRange);
  EXPECT_EQ(inexact.error(), UnitConversionError::InexactConversion);
}

// =============================================================================
// =============================================================================

TEST(GGEMSUnitRegistryContractTest, BridgesBitsAndBytesExplicitlyAndExactly) {
  auto const bits = TryConvertBytesToBits(Bytes{2ULL});
  auto const bytes = TryConvertBitsToBytes(Bits{16ULL});
  auto const inexact_bytes = TryConvertBitsToBytes(Bits{7ULL});
  auto const overflowing_bits =
      TryConvertBytesToBits(Bytes{std::numeric_limits<std::uint64_t>::max()});

  ASSERT_TRUE(bits.has_value());
  ASSERT_TRUE(bytes.has_value());
  ASSERT_FALSE(inexact_bytes.has_value());
  ASSERT_FALSE(overflowing_bits.has_value());
  EXPECT_EQ(bits->value, 16ULL);
  EXPECT_EQ(bytes->value, 2ULL);
  EXPECT_EQ(inexact_bytes.error(), UnitConversionError::InexactConversion);
  EXPECT_EQ(overflowing_bits.error(), UnitConversionError::OutOfRange);
}

// =============================================================================
// =============================================================================

TEST(GGEMSUnitRegistryContractTest, AppliesFamilySpecificDisplayPolicies) {
  EXPECT_EQ(HumanReadable(Bytes{1'024ULL}), "1.0000000 KiB");
  EXPECT_EQ(HumanReadable(Bits{1'000ULL}), "1.0000000 kbit");
  EXPECT_EQ(HumanReadable(1_cm), "10.0000000 mm");
  EXPECT_EQ(HumanReadable(PositionCoordinate{-1'000'000LL}), "-1.0000000 µm");
  EXPECT_EQ(HumanReadable(Dose{1ULL}), "0.1602177 µGy");
  EXPECT_EQ(HumanReadable(90_s), "1 min 30 s 0 ms");
  EXPECT_EQ(HumanReadable(12.3456_deg, 2), "12.35 deg");
  EXPECT_EQ(HumanReadable(12.3456_deg, 1), "12.3 deg");
  EXPECT_EQ(std::format("{:>20}", Length{1ULL}), "        1.0000000 pm");
}

} // namespace
