#pragma once

#include <cstdint>
#include <format>
#include <string>
#include <type_traits>

namespace ggems::units {
// -----------------------------
// Dimension: L^Lexp T^Texp I^Iexp
// L = Length, T = Time, I = Information
// -----------------------------
template <int LExp, int TExp, int IExp> struct Dim {};

// Base dimensions
using LengthDim = Dim<1, 0, 0>;
using TimeDim = Dim<0, 1, 0>;
using InfoBytesDim = Dim<0, 0, 1>;
using InfoBitsDim = Dim<0, 0, 2>;
using FrequencyDim = Dim<0, -1, 0>;
using SpeedDim = Dim<1, -1, 0>;
using BandwidthDim = Dim<0, -1, 1>;

// -----------------------------
// Generic quantity
// -----------------------------
template <typename DimT, typename Rep = std::uint64_t> struct Quantity {
  Rep value{};

  constexpr auto operator<=>(Quantity const &) const = default;
};

// -----------------------------
// Dimension division: D1 / D2
// -----------------------------
template <typename D1, typename D2> struct DivDim;

template <int L1, int T1, int I1, int L2, int T2, int I2>
struct DivDim<Dim<L1, T1, I1>, Dim<L2, T2, I2>> {
  using type = Dim<L1 - L2, T1 - T2, I1 - I2>;
};

// -----------------------------
// Addition / subtraction (same dimension)
// -----------------------------
template <typename D, typename R1, typename R2>
constexpr auto operator+(Quantity<D, R1> lhs, Quantity<D, R2> rhs) noexcept {
  using OutRep = std::common_type_t<R1, R2>;
  return Quantity<D, OutRep>{static_cast<OutRep>(lhs.value) +
                             static_cast<OutRep>(rhs.value)};
}

template <typename D, typename R1, typename R2>
constexpr auto operator-(Quantity<D, R1> lhs, Quantity<D, R2> rhs) noexcept {
  using OutRep = std::common_type_t<R1, R2>;
  return Quantity<D, OutRep>{static_cast<OutRep>(lhs.value) -
                             static_cast<OutRep>(rhs.value)};
}

// -----------------------------
// Division: Quantity<D1> / Quantity<D2>
// Result dimension = DivDim<D1,D2>
// -----------------------------
template <typename D1, typename R1, typename D2, typename R2>
constexpr auto operator/(Quantity<D1, R1> lhs, Quantity<D2, R2> rhs) {
  using OutDim = typename DivDim<D1, D2>::type;
  using OutRep = std::common_type_t<R1, R2, long double>;
  return Quantity<OutDim, OutRep>{static_cast<OutRep>(lhs.value) /
                                  static_cast<OutRep>(rhs.value)};
}

template <typename DimT, typename Rep, typename Scalar,
          std::enable_if_t<std::is_arithmetic_v<Scalar>, int> = 0>
constexpr auto operator*(Quantity<DimT, Rep> q, Scalar s) noexcept {
  using OutRep = std::common_type_t<Rep, Scalar>;
  return Quantity<DimT, OutRep>{static_cast<OutRep>(q.value) *
                                static_cast<OutRep>(s)};
}

template <typename DimT, typename Rep, typename Scalar,
          std::enable_if_t<std::is_arithmetic_v<Scalar>, int> = 0>
constexpr auto operator*(Scalar s, Quantity<DimT, Rep> q) noexcept {
  using OutRep = std::common_type_t<Rep, Scalar>;
  return Quantity<DimT, OutRep>{static_cast<OutRep>(q.value) *
                                static_cast<OutRep>(s)};
}

// -----------------------------
// Generic HumanReadable fallback
// Specialisations/overloads will handle Time, Length, etc.
// -----------------------------
template <typename D, typename R>
inline std::string HumanReadable(Quantity<D, R> const &q) {
  return std::format("{}", q.value);
}

} // namespace ggems::units

// --------------------------------------------------------
// Generic formatter using HumanReadable via ADL
// --------------------------------------------------------
namespace std {
template <typename DimT, typename Rep>
struct formatter<ggems::units::Quantity<DimT, Rep>>
    : std::formatter<std::string> {
  auto format(ggems::units::Quantity<DimT, Rep> const &q, auto &ctx) const {
    using ggems::units::HumanReadable;
    return std::formatter<std::string>::format(HumanReadable(q), ctx);
  }
};
} // namespace std
