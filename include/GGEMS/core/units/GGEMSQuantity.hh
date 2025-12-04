#pragma once
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

/*!
 * \file GGEMSQuantity.hh
 * \brief Dimensional quantity system and physical unit base types.
 *
 * This header defines GGEMS core system for representing physical
 * quantities with explicit dimensional exponents. A quantity is modelled
 * as a compile-time dimension type \c Dim with exponents applied to the
 * base dimensions of Length (L), Time (T) and Information (I), together
 * with a numeric representation holding the scaled integer value.
 *
 * \par Purpose
 * The mechanism enforces dimensional consistency in expressions, prevents
 * invalid mixes of unrelated units at compile time, and enables exact
 * integer-based storage for GGEMS' internal physical unit system.
 *
 * \par Design principles
 *  - Dimensions are specified as integer exponents on base primitives:
 *    \c Length, \c Time and \c Information.
 *  - Quantities are strongly typed via \c Quantity<Dim,R> where \c Dim
 *    encodes the dimension and \c R holds the numeric representation.
 *  - Arithmetic rules preserve dimensional correctness: addition and
 *    subtraction require matching dimensions, while division of
 *    quantities derives a new dimension according to exponent algebra.
 *
 * Human-readable representations are produced via \c HumanReadable, and
 * formatting is integrated into \c std::format by a formatter
 * specialisation.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

/// \cond
#include <format>
/// \endcond

namespace ggems::units {

/*!
 * \brief Compile-time dimensional type.
 *
 * The \c Dim type encodes physical dimensionality as integer exponents
 * applied to Length (LExp), Time (TExp), Mass (MExp) and Information (IExp).
 *
 * Examples:
 *  - \c Dim<1,0,0,0>  ⇒ length
 *  - \c Dim<0,1,0,0>  ⇒ time
 *  - \c Dim<1,-1,0,0> ⇒ speed (length/time)
 *
 * \tparam LExp Exponent for Length.
 * \tparam TExp Exponent for Time.
 * \tparam MExp Exponent for Mass
 * \tparam IExp Exponent for Information.
 */
template <std::int8_t LExp, std::int8_t TExp, std::int8_t MExp,
          std::int8_t IExp>
struct Dim {};

/*! \brief Base dimension for length (L). */
using LengthDim = Dim<1, 0, 0, 0>;
/*! \brief Dimension for area (L²) */
using AreaDim = Dim<2, 0, 0, 0>;
/*! \brief Dimension for volume (L³) */
using VolumeDim = Dim<3, 0, 0, 0>;
/*! \brief Base dimension for time (T). */
using TimeDim = Dim<0, 1, 0, 0>;
/*! \brief Base dimension for mass (M). */
using MassDim = Dim<0, 0, 1, 0>;
/*! \brief Base dimension for information measured in bytes. */
using InfoBytesDim = Dim<0, 0, 0, 1>;
/*! \brief Base dimension for information measured in bits. */
using InfoBitsDim = Dim<0, 0, 0, 2>;
/*! \brief Dimension for frequency (T⁻¹). */
using FrequencyDim = Dim<0, -1, 0, 0>;
/*! \brief Dimension for speed (L T⁻¹). */
using SpeedDim = Dim<1, -1, 0, 0>;
/*! \brief Dimension for bandwidth (information per unit time). */
using BandwidthDim = Dim<0, -1, 0, 1>;
/*! \brief Dimension for density (M L⁻³) */
using DensityDim = Dim<-3, 0, 1, 0>;
/*! \brief Dimension for energy (L²T⁻²M) */
using EnergyDim = Dim<2, -2, 1, 0>;
/*! \brief Dimension for dose (L²T⁻²) */
using DoseDim = Dim<2, -2, 0, 0>;

/*!
 * \brief Strongly-typed physical quantity with an associated dimension.
 *
 * \tparam DimT Compile-time dimension type (e.g. \c LengthDim, \c TimeDim).
 * \tparam Rep  Underlying representation type used to store the value.
 *
 * The stored \c value is expressed in GGEMS base engine units for the
 * corresponding quantity family (e.g. picoseconds for time, picometres
 * for length). Higher-level unit headers define aliases such as
 * \c Time or \c Length bound to concrete representations.
 */
template <typename DimT, typename Rep = std::uint64_t> struct Quantity {
  Rep value{}; /*!< Stored scalar value in base engine units. */

  /*!
   * \brief Default three-way comparison between quantities.
   *
   * Quantities of the same dimension are ordered according to their
   * stored scalar \c value. Mixed-dimension comparisons are ill-formed
   * at compile time.
   * \return Strong ordering result that reflects the relative ordering
   *         of the underlying raw values.
   */
  constexpr auto operator<=>(Quantity const &) const = default;
};

/*!
 * \brief Metafunction computing the dimension of a quotient of quantities.
 *
 * Given two dimension types \c D1 and \c D2, the nested \c type alias
 * of \c DivDim<D1,D2> represents the dimension of the quotient
 * \f$ D_1 / D_2 \f$ obtained by subtracting each exponent component-wise.
 *
 * Specialisations are provided for \c Dim.
 *
 * \tparam D1 Numerator dimension type.
 * \tparam D2 Denominator dimension type.
 */
template <typename D1, typename D2> struct DivDim;

/*!
 * \brief Specialisation of DivDim for \c Dim exponents.
 *
 * \tparam L1 Length exponent of numerator.
 * \tparam T1 Time exponent of numerator.
 * \tparam M1 Mass exponent of numerator.
 * \tparam I1 Information exponent of numerator.
 * \tparam L2 Length exponent of denominator.
 * \tparam T2 Time exponent of denominator.
 * \tparam M2 Mass exponent of denominator.
 * \tparam I2 Information exponent of denominator.
 */
template <std::int8_t L1, std::int8_t T1, std::int8_t M1, std::int8_t I1,
          std::int8_t L2, std::int8_t T2, std::int8_t M2, std::int8_t I2>
struct DivDim<Dim<L1, T1, M1, I1>, Dim<L2, T2, M2, I2>> {
  using type =
      Dim<L1 - L2, T1 - T2, M1 - M2, I1 - I2>; /*!< Resulting dimension. */
};

/*!
 * \brief Adds two quantities with identical dimensions.
 *
 * The result uses the common type of the two representations. The
 * operation is only defined when the dimensions match, enforcing
 * dimensional correctness at compile time.
 *
 * \tparam D  Dimension type (same for both operands).
 * \tparam R1 Representation type of the left-hand side.
 * \tparam R2 Representation type of the right-hand side.
 * \param lhs Left-hand side quantity.
 * \param rhs Right-hand side quantity.
 * \return Quantity with dimension \c D and common representation type.
 */
template <typename D, typename R1, typename R2>
constexpr auto operator+(Quantity<D, R1> lhs, Quantity<D, R2> rhs) noexcept {
  using OutRep = std::common_type_t<R1, R2>;
  return Quantity<D, OutRep>{static_cast<OutRep>(lhs.value) +
                             static_cast<OutRep>(rhs.value)};
}

/*!
 * \brief Subtracts two quantities with identical dimensions.
 *
 * The result uses the common type of the two representations. As for
 * addition, dimensions must match to keep the operation well-formed.
 *
 * \tparam D  Dimension type (same for both operands).
 * \tparam R1 Representation type of the left-hand side.
 * \tparam R2 Representation type of the right-hand side.
 * \param lhs Left-hand side quantity.
 * \param rhs Right-hand side quantity.
 * \return Quantity with dimension \c D and common representation type.
 */
template <typename D, typename R1, typename R2>
constexpr auto operator-(Quantity<D, R1> lhs, Quantity<D, R2> rhs) noexcept {
  using OutRep = std::common_type_t<R1, R2>;
  return Quantity<D, OutRep>{static_cast<OutRep>(lhs.value) -
                             static_cast<OutRep>(rhs.value)};
}

/*!
 * \brief Divides two quantities and produces a quantity of derived dimension.
 *
 * The resulting dimension is obtained via \c DivDim<D1,D2>::type and the
 * representation type is the common type of both operands extended with
 * \c long double to keep precision for non-integer ratios.
 *
 * \tparam D1 Dimension of the numerator.
 * \tparam R1 Representation of the numerator.
 * \tparam D2 Dimension of the denominator.
 * \tparam R2 Representation of the denominator.
 * \param lhs Numerator quantity.
 * \param rhs Denominator quantity.
 * \return Quantity with derived dimension and floating-point representation.
 */
template <typename D1, typename R1, typename D2, typename R2>
constexpr auto operator/(Quantity<D1, R1> lhs, Quantity<D2, R2> rhs) noexcept {
  using OutDim = typename DivDim<D1, D2>::type;
  using OutRep = std::common_type_t<R1, R2, long double>;
  return Quantity<OutDim, OutRep>{static_cast<OutRep>(lhs.value) /
                                  static_cast<OutRep>(rhs.value)};
}

/*!
 * \brief Multiplies a quantity by a scalar factor (quantity * scalar).
 *
 * \tparam DimT  Dimension type of the quantity.
 * \tparam Rep   Representation type of the quantity.
 * \tparam Scalar Arithmetic scalar type.
 * \param q Quantity to scale.
 * \param s Scalar factor.
 * \return Quantity with unchanged dimension and common representation type.
 */
template <typename DimT, typename Rep, typename Scalar,
          std::enable_if_t<std::is_arithmetic_v<Scalar>, int> = 0>
constexpr auto operator*(Quantity<DimT, Rep> q, Scalar s) noexcept {
  using OutRep = std::common_type_t<Rep, Scalar>;
  return Quantity<DimT, OutRep>{static_cast<OutRep>(q.value) *
                                static_cast<OutRep>(s)};
}

/*!
 * \brief Multiplies a quantity by a scalar factor (scalar * quantity).
 *
 * This overload is symmetric to the quantity–scalar version and enables
 * natural expression ordering.
 *
 * \tparam DimT  Dimension type of the quantity.
 * \tparam Rep   Representation type of the quantity.
 * \tparam Scalar Arithmetic scalar type.
 * \param s Scalar factor.
 * \param q Quantity to scale.
 * \return Quantity with unchanged dimension and common representation type.
 */
template <typename DimT, typename Rep, typename Scalar,
          std::enable_if_t<std::is_arithmetic_v<Scalar>, int> = 0>
constexpr auto operator*(Scalar s, Quantity<DimT, Rep> q) noexcept {
  using OutRep = std::common_type_t<Rep, Scalar>;
  return Quantity<DimT, OutRep>{static_cast<OutRep>(q.value) *
                                static_cast<OutRep>(s)};
}

/*!
 * \brief Generic fallback for human-readable quantity formatting.
 *
 * This overload simply returns the stored \c value as a decimal string.
 * Unit-specific headers such as \c GGEMSTimeUnits.hh can provide more
 * specialised overloads that select appropriate display units and
 * suffixes while keeping this generic version as a safe default.
 *
 * \tparam D Dimension type of the quantity.
 * \tparam R Representation type of the quantity.
 * \param q Quantity to format.
 * \return UTF-8 encoded decimal representation of \c q.value.
 */
template <typename D, typename R>
inline std::string HumanReadable(Quantity<D, R> const &q) {
  return std::format("{}", q.value);
}

} // namespace ggems::units

/// \cond
namespace std {
template <typename DimT, typename Rep>
struct formatter<ggems::units::Quantity<DimT, Rep>> : formatter<string> {
  auto format(ggems::units::Quantity<DimT, Rep> const &q, auto &ctx) const {
    using ggems::units::HumanReadable;
    return formatter<string>::format(HumanReadable(q), ctx);
  }
};
} // namespace std
/// \endcond
