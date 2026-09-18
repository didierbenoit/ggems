#pragma once

#include <algorithm>
#include <cmath>
#include <format>
#include <string_view>
#include <vector>

#include "GGEMS/GGEMSException.hh"

namespace ggems::core::materials::detail {

inline constexpr long double k_provisional_fraction_sum_tolerance{1.0e-5L};

template <typename Entry, typename KeyProjection>
auto CanonicalizeFractions(std::vector<Entry> &entries,
                           KeyProjection key_projection,
                           long double Entry::*fraction_member,
                           std::string_view subject) -> void {
  for (auto const &entry : entries) {
    long double const fraction = entry.*fraction_member;

    if (!std::isfinite(fraction) || fraction < 0.0L) {
      throw GGEMSRecoverable{
          std::format("{} fractions must be finite and nonnegative.", subject)};
    }
  }

  std::ranges::sort(entries, {}, key_projection);

  if (std::ranges::adjacent_find(entries, {}, key_projection) !=
      entries.end()) {
    throw GGEMSRecoverable{
        std::format("{} fractions contain duplicate keys.", subject)};
  }

  std::erase_if(entries, [fraction_member](Entry const &entry) -> bool {
    return entry.*fraction_member == 0.0L;
  });

  if (entries.empty()) {
    throw GGEMSRecoverable{std::format(
        "{} fractions must contain at least one positive fraction.", subject)};
  }

  long double fraction_sum{0.0L};
  for (auto const &entry : entries) {
    fraction_sum += entry.*fraction_member;
  }

  if (!std::isfinite(fraction_sum) ||
      std::abs(fraction_sum - 1.0L) > k_provisional_fraction_sum_tolerance) {
    throw GGEMSRecoverable{std::format(
        "{} fractions must sum to one within the provisional 1.0e-5 "
        "admission tolerance.",
        subject)};
  }

  for (auto &entry : entries) {
    entry.*fraction_member /= fraction_sum;

    if (!std::isnormal(entry.*fraction_member)) {
      throw GGEMSRecoverable{std::format(
          "{} fractions must be normal floating-point values.", subject)};
    }
  }
}

} // namespace ggems::core::materials::detail
