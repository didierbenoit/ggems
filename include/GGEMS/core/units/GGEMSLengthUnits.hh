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
 * \file GGEMSLengthUnits.hh
 * \brief Length quantity and formatting utilities for GGEMS.
 *
 * Defines the \c Length quantity stored in engine base units
 * (picometres). User-defined literals provide conversions from common
 * physical scales (nm → km), and a helper function emits readable
 * textual representations with SI prefixes.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-29
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 */

/// \cond
#include <format>
#include <array>
#include <cstdint>
#include <string>
#include <string_view>
/// \endcond

#include "GGEMS/core/units/GGEMSQuantity.hh"

namespace ggems::units {
/*!
 * \brief Length quantity expressed in picometres (pm).
 *
 * This alias binds the \c LengthDim dimension to an unsigned 64-bit
 * representation. All stored values correspond to engine base units in
 * picometres. Higher-level user-facing scales (nm → km) are produced
 * only by \c HumanReadable for display convenience.
 */
using Length = Quantity<LengthDim, std::uint64_t>;

/*!
 * \brief Converts a length quantity to a human-readable UTF-8 string.
 *
 * Internal values are stored as picometres (pm). For readability, the
 * function selects the most appropriate SI prefix (nm, μm, mm, m, km)
 * based on magnitude and formats the scaled value using fixed precision
 * and optional minimum width.
 *
 * \param l         Length quantity expressed in picometres.
 * \param precision Digits after the decimal point.
 * \param width     Minimum formatted width; if negative, no constraint.
 *
 * \return UTF-8 encoded textual representation with an SI suffix.
 */
inline auto HumanReadable(Length const &length, std::int8_t precision = 7,
                          std::int8_t width = -1) -> std::string {
  auto value = static_cast<long double>(length.value);

  struct Unit {
    long double threshold;
    std::string_view suffix;
    long double scale;
  };

  static constexpr std::array<Unit, 6> units{
      {{.threshold = 1.0e15L, .suffix = " km", .scale = 1.0e15L},
       {.threshold = 1.0e12L, .suffix = " m", .scale = 1.0e12L},
       {.threshold = 1.0e9L, .suffix = " mm", .scale = 1.0e9L},
       {.threshold = 1.0e6L, .suffix = " um", .scale = 1.0e6L},
       {.threshold = 1.0e3L, .suffix = " nm", .scale = 1.0e3L},
       {.threshold = 0.0L, .suffix = " pm", .scale = 1.0L}}};

  for (auto const &unit : units) {
    if (value >= unit.threshold) {

      long double scaled = value / unit.scale;

      std::string fmt;

      if (width < 0) {
        fmt = std::format("{{:.{}f}}{}", precision, unit.suffix);
      } else {
        fmt = std::format("{{:{}.{}f}}{}", width, precision, unit.suffix);
      }

      return std::vformat(fmt, std::make_format_args(scaled));
    }
  }

  return std::format("{:.{}f} pm", value, precision);
}

[[nodiscard]] inline auto
HumanReadableSignedLength(std::int64_t const value_pm,
                          std::int8_t const precision = 7,
                          std::int8_t const width = -1) -> std::string {
  if (value_pm >= 0LL) {
    return HumanReadable(Length{static_cast<std::uint64_t>(value_pm)},
                         precision, width);
  }

  std::uint64_t magnitude_pm =
      static_cast<std::uint64_t>(-(value_pm + 1LL)) + 1ULL;

  return "-" + HumanReadable(Length{magnitude_pm}, precision, width);
}

/*!
 * \brief User-defined literal for picometres (pm).
 * \param v Integer literal in pm.
 * \return \c Length quantity equal to \c v pm.
 */
consteval auto operator""_pm(unsigned long long value) noexcept -> Length {
  return Length{static_cast<std::uint64_t>(value)};
}

/*!
 * \brief User-defined literal for picometres (pm) from floating-point.
 * \param v Floating-point literal in pm.
 * \return \c Length quantity approximating \c v pm (fraction truncated).
 */
consteval auto operator""_pm(long double value) noexcept -> Length {
  return Length{static_cast<std::uint64_t>(value)};
}

/*!
 * \brief User-defined literal for nanometres (nm).
 * \param v Integer literal in nm.
 * \return \c Length quantity equal to \c v × 10³ pm.
 */
consteval auto operator""_nm(unsigned long long value) noexcept -> Length {
  return Length{static_cast<std::uint64_t>(value) * 1000ULL};
}

/*!
 * \brief User-defined literal for nanometres (nm) from floating-point.
 * \param v Floating-point literal in nm.
 * \return \c Length quantity approximating \c v × 10³ pm.
 */
consteval auto operator""_nm(long double value) noexcept -> Length {
  return Length{static_cast<std::uint64_t>(value * 1.0e3L)};
}

/*!
 * \brief User-defined literal for micrometres (μm).
 * \param v Integer literal in um.
 * \return \c Length quantity equal to \c v × 10⁶ pm.
 */
consteval auto operator""_um(unsigned long long value) noexcept -> Length {
  return Length{static_cast<std::uint64_t>(value) * 1'000'000ULL};
}

/*!
 * \brief User-defined literal for micrometres (μm) from floating-point.
 * \param v Floating-point literal in um.
 * \return \c Length quantity approximating \c v × 10⁶ pm.
 */
consteval auto operator""_um(long double value) noexcept -> Length {
  return Length{static_cast<std::uint64_t>(value * 1.0e6L)};
}

/*!
 * \brief User-defined literal for millimetres (mm).
 * \param v Integer literal in mm.
 * \return \c Length quantity equal to \c v × 10⁹ pm.
 */
consteval auto operator""_mm(unsigned long long value) noexcept -> Length {
  return Length{static_cast<std::uint64_t>(value) * 1'000'000'000ULL};
}

/*!
 * \brief User-defined literal for millimetres (mm) from floating-point.
 * \param v Floating-point literal in mm.
 * \return \c Length quantity approximating \c v × 10⁹ pm.
 */
consteval auto operator""_mm(long double value) noexcept -> Length {
  return Length{static_cast<std::uint64_t>(value * 1.0e9L)};
}

/*!
 * \brief User-defined literal for metres (m).
 * \param v Integer literal in m.
 * \return \c Length quantity equal to \c v × 10¹² pm.
 */
consteval auto operator""_m(unsigned long long value) noexcept -> Length {
  return Length{static_cast<std::uint64_t>(value) * 1'000'000'000'000ULL};
}

/*!
 * \brief User-defined literal for metres (m) from floating-point.
 * \param v Floating-point literal in m.
 * \return \c Length quantity approximating \c v × 10¹² pm.
 */
consteval auto operator""_m(long double value) noexcept -> Length {
  return Length{static_cast<std::uint64_t>(value * 1.0e12L)};
}

/*!
 * \brief User-defined literal for kilometres (km).
 *
 * \param v Integer literal in kilometres.
 * \return \c Length quantity equal to \c v × 10¹⁵ pm.
 */
consteval auto operator""_km(unsigned long long value) noexcept -> Length {
  return Length{static_cast<std::uint64_t>(value) * 1'000'000'000'000'000ULL};
}

/*!
 * \brief User-defined literal for kilometres (km) from floating-point.
 *
 * \param v Floating-point literal in kilometres.
 * \return \c Length quantity approximating \c v × 10¹⁵ pm.
 */
consteval auto operator""_km(long double value) noexcept -> Length {
  return Length{static_cast<std::uint64_t>(value * 1.0e15L)};
}
} // namespace ggems::units
