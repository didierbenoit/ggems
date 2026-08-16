#pragma once

#include <concepts>
#include <type_traits>

namespace ggems::units {

template <typename Tag, typename Representation> struct Quantity {
  using tag = Tag;
  using representation = Representation;
  Representation value{};
  constexpr auto operator<=>(Quantity const &) const = default;
};

template <typename Type> struct IsQuantity : std::false_type {};

template <typename Tag, typename Representation>
struct IsQuantity<Quantity<Tag, Representation>> : std::true_type {};

template <typename Type>
concept QuantityType = IsQuantity<std::remove_cvref_t<Type>>::value;

template <typename Type>
concept Arithmetic = std::integral<Type> || std::floating_point<Type>;

template <typename Tag, typename Representation>
constexpr auto operator+(Quantity<Tag, Representation> lhs,
                         Quantity<Tag, Representation> rhs) noexcept
    -> Quantity<Tag, Representation> {
  return {lhs.value + rhs.value};
}

template <typename Tag, typename Representation>
constexpr auto operator-(Quantity<Tag, Representation> lhs,
                         Quantity<Tag, Representation> rhs) noexcept
    -> Quantity<Tag, Representation> {
  return {lhs.value - rhs.value};
}

template <typename Tag, typename Representation>
  requires std::signed_integral<Representation> ||
           std::floating_point<Representation>
constexpr auto operator-(Quantity<Tag, Representation> quantity) noexcept
    -> Quantity<Tag, Representation> {
  return {-quantity.value};
}

template <typename Tag, typename Representation, Arithmetic Scalar>
  requires std::floating_point<Representation>
constexpr auto operator*(Quantity<Tag, Representation> quantity,
                         Scalar scale) noexcept
    -> Quantity<Tag, Representation> {
  return {quantity.value * static_cast<long double>(scale)};
}

template <typename Tag, typename Representation, Arithmetic Scalar>
  requires std::floating_point<Representation>
constexpr auto operator*(Scalar scale,
                         Quantity<Tag, Representation> quantity) noexcept
    -> Quantity<Tag, Representation> {
  return quantity * scale;
}

template <typename Tag, typename Representation, Arithmetic Scalar>
  requires std::floating_point<Representation>
constexpr auto operator/(Quantity<Tag, Representation> quantity,
                         Scalar scale) noexcept
    -> Quantity<Tag, Representation> {
  return {quantity.value / static_cast<long double>(scale)};
}

} // namespace ggems::units
