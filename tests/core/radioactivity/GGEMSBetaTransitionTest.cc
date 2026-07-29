#include <array>

#include <gtest/gtest.h>

#include "GGEMS/core/GGEMSException.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaSpectrumBuilder.hh"
#include "GGEMS/core/radioactivity/GGEMSBetaTransition.hh"

namespace {

// =============================================================================
// =============================================================================

using ggems::core::GGEMSExceptionBase;
using ggems::core::radioactivity::BuildBetaSpectrum;
using ggems::core::radioactivity::GGEMSBetaSign;
using ggems::core::radioactivity::GGEMSBetaTransition;
using ggems::core::radioactivity::GGEMSBetaTransitionClass;

// =============================================================================
// =============================================================================

[[nodiscard]] auto MakeTransition(GGEMSBetaSign sign = GGEMSBetaSign::Minus,
                                  GGEMSBetaTransitionClass transition_class =
                                      GGEMSBetaTransitionClass::Allowed)
    -> GGEMSBetaTransition {
  return {sign, 8U, 18U, 633'900'000ULL, transition_class};
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaTransitionTest, OwnsValidMinusAndPlusValues) {
  GGEMSBetaTransition const minus = MakeTransition(GGEMSBetaSign::Minus);
  GGEMSBetaTransition const plus = MakeTransition(GGEMSBetaSign::Plus);

  EXPECT_EQ(minus.GetSign(), GGEMSBetaSign::Minus);
  EXPECT_EQ(plus.GetSign(), GGEMSBetaSign::Plus);
  EXPECT_EQ(plus.GetDaughterAtomicNumber(), 8U);
  EXPECT_EQ(plus.GetDaughterMassNumber(), 18U);
  EXPECT_EQ(plus.GetEndpointKineticEnergyMilliElectronVolt(), 633'900'000ULL);
  EXPECT_EQ(plus.GetTransitionClass(), GGEMSBetaTransitionClass::Allowed);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaTransitionTest, RejectsInvalidPhysicalFields) {
  EXPECT_THROW(((void)GGEMSBetaTransition{GGEMSBetaSign::Minus, 0U, 18U, 1ULL,
                                          GGEMSBetaTransitionClass::Allowed}),
               GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSBetaTransition{GGEMSBetaSign::Minus, 8U, 0U, 1ULL,
                                          GGEMSBetaTransitionClass::Allowed}),
               GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSBetaTransition{GGEMSBetaSign::Minus, 8U, 7U, 1ULL,
                                          GGEMSBetaTransitionClass::Allowed}),
               GGEMSExceptionBase);
  EXPECT_THROW(((void)GGEMSBetaTransition{GGEMSBetaSign::Minus, 8U, 18U, 0ULL,
                                          GGEMSBetaTransitionClass::Allowed}),
               GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaTransitionTest, RejectsUnknownEnumValues) {
  EXPECT_THROW(
      ((void)GGEMSBetaTransition{static_cast<GGEMSBetaSign>(255U), 8U, 18U,
                                 1ULL, GGEMSBetaTransitionClass::Allowed}),
      GGEMSExceptionBase);
  EXPECT_THROW(
      ((void)GGEMSBetaTransition{GGEMSBetaSign::Minus, 8U, 18U, 1ULL,
                                 static_cast<GGEMSBetaTransitionClass>(255U)}),
      GGEMSExceptionBase);
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaTransitionTest,
     RetainsKnownNonAllowedClassesButBuilderRejectsEach) {
  constexpr std::array<GGEMSBetaTransitionClass, 7U> unsupported{
      GGEMSBetaTransitionClass::FirstForbidden,
      GGEMSBetaTransitionClass::UniqueFirstForbidden,
      GGEMSBetaTransitionClass::SecondForbidden,
      GGEMSBetaTransitionClass::UniqueSecondForbidden,
      GGEMSBetaTransitionClass::ThirdForbidden,
      GGEMSBetaTransitionClass::UniqueThirdForbidden,
      GGEMSBetaTransitionClass::Unclassified};

  for (GGEMSBetaTransitionClass transition_class : unsupported) {
    SCOPED_TRACE(static_cast<unsigned int>(transition_class));
    GGEMSBetaTransition const transition =
        MakeTransition(GGEMSBetaSign::Minus, transition_class);
    EXPECT_EQ(transition.GetTransitionClass(), transition_class);
    EXPECT_THROW((void)BuildBetaSpectrum(transition), GGEMSExceptionBase);
  }
}

// =============================================================================
// =============================================================================

TEST(GGEMSBetaTransitionTest, HasIndependentValueSemantics) {
  GGEMSBetaTransition const original = MakeTransition(GGEMSBetaSign::Plus);
  GGEMSBetaTransition copy = original;
  EXPECT_EQ(copy, original);

  copy = GGEMSBetaTransition{GGEMSBetaSign::Minus, 83U, 210U, 1'160'000'000ULL,
                             GGEMSBetaTransitionClass::Allowed};

  EXPECT_EQ(original.GetSign(), GGEMSBetaSign::Plus);
  EXPECT_EQ(original.GetDaughterAtomicNumber(), 8U);
  EXPECT_EQ(original.GetEndpointKineticEnergyMilliElectronVolt(),
            633'900'000ULL);
  EXPECT_NE(copy, original);
}

} // namespace
