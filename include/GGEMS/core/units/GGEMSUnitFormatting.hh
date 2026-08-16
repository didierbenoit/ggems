#pragma once

#include <cmath>
#include <cstdint>
#include <format>
#include <limits>
#include <string>
#include <string_view>

#include "GGEMS/core/GGEMSLogger.hh"
#include "GGEMS/core/units/GGEMSQuantity.hh"
#include "GGEMS/core/units/GGEMSUnitConversion.hh"

namespace ggems::units {

namespace detail {

[[nodiscard]] inline auto SelectUnitSymbol(UnitDefinition const &unit) noexcept
    -> std::string_view {
  if (ggems::core::GGEMSLogger::GetInstance().GetEncoding() ==
      ggems::core::Encoding::Ascii) {
    return unit.symbol;
  }
  return unit.unicode_symbol.empty() ? unit.symbol : unit.unicode_symbol;
}

template <typename QuantityType>
auto FormatScaled(QuantityType const &quantity, UnitDefinition const &unit,
                  std::int8_t precision, std::int8_t width) -> std::string {
  long double const scaled =
      static_cast<long double>(quantity.value) / ScaleFactor(unit.scale);

  std::string_view const selected_symbol = SelectUnitSymbol(unit);
  std::string format;
  if (width < 0) {
    format = std::format("{{:.{}f}} {}", precision, selected_symbol);
  } else {
    format = std::format("{{:{}.{}f}} {}", width, precision, selected_symbol);
  }
  return std::vformat(format, std::make_format_args(scaled));
}

} // namespace detail

template <QuantityType QuantityValue>
auto HumanReadable(
    QuantityValue const &quantity,
    std::int8_t precision =
        QuantityTraits<typename QuantityValue::tag>::default_precision,
    std::int8_t width = -1) -> std::string {
  using Traits = QuantityTraits<typename QuantityValue::tag>;
  using UnitSet = typename Traits::unit_set;
  if constexpr (Traits::format_policy == QuantityFormatPolicy::FixedUnit) {
    return detail::FormatScaled(quantity,
                                *FindUnit<UnitSet>(Traits::fixed_display_unit),
                                precision, width);
  } else {
    if constexpr (Traits::format_policy ==
                  QuantityFormatPolicy::DurationBreakdown) {
      auto const second_factor = static_cast<std::uint64_t>(
          detail::ScaleFactor(FindUnit<UnitSet>("s")->scale));
      if (quantity.value >= 60ULL * second_factor) {
        auto const millisecond_factor = static_cast<std::uint64_t>(
            detail::ScaleFactor(FindUnit<UnitSet>("ms")->scale));
        auto const total_seconds = quantity.value / second_factor;
        auto const remainder = quantity.value % second_factor;
        auto const hours = total_seconds / 3'600ULL;
        auto const minutes = total_seconds % 3'600ULL / 60ULL;
        auto const seconds = total_seconds % 60ULL;
        auto const milliseconds = remainder / millisecond_factor;
        if (hours > 0ULL) {
          return std::format("{} h {} min {} s {} ms", hours, minutes, seconds,
                             milliseconds);
        }
        return std::format("{} min {} s {} ms", minutes, seconds, milliseconds);
      }
    }

    long double const magnitude =
        std::abs(static_cast<long double>(quantity.value));

    UnitDefinition const *selected{nullptr};
    long double selected_factor{-1.0L};
    for (auto const &unit : UnitRegistry<UnitSet>::units) {
      long double const factor = detail::ScaleFactor(unit.scale);
      if (unit.automatic_display && magnitude >= factor &&
          factor > selected_factor) {
        selected = &unit;
        selected_factor = factor;
      }
    }

    if (selected == nullptr) {
      long double smallest_factor = std::numeric_limits<long double>::max();
      for (auto const &unit : UnitRegistry<UnitSet>::units) {
        long double const factor = detail::ScaleFactor(unit.scale);
        if (unit.automatic_display && factor < smallest_factor) {
          selected = &unit;
          smallest_factor = factor;
        }
      }
    }
    return detail::FormatScaled(quantity, *selected, precision, width);
  }
}

} // namespace ggems::units

namespace std {
template <typename Tag, typename Representation>
struct formatter<ggems::units::Quantity<Tag, Representation>>
    : formatter<string> {
  template <typename FormatContext>
  auto format(ggems::units::Quantity<Tag, Representation> const &quantity,
              FormatContext &context) const ->
      typename FormatContext::iterator {
    return formatter<string>::format(ggems::units::HumanReadable(quantity),
                                     context);
  }
};
} // namespace std
