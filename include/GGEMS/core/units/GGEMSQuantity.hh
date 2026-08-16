// *****************************************************************************
// * This file is part of GGEMS.                                               *
// *                                                                           *
// * SPDX-License-Identifier: GPL-3.0-or-later                                 *
// * Copyright (C) 2017-2026 CHRU de Brest, Université de Bretagne Occidentale,*
// * Inserm.                                                                   *
// *                                                                           *
// * GGEMS is free software: you can redistribute it and/or modify             *
// * it under the terms of the GNU General Public License as published by      *
// * the Free Software Foundation, either version 3 of the License, or         *
// * (at your option) any later version.                                       *
// *                                                                           *
// * GGEMS is distributed in the hope that it will be useful,                  *
// * but WITHOUT ANY WARRANTY; without even the implied warranty of            *
// * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the              *
// * GNU General Public License for more details.                              *
// *                                                                           *
// * You should have received a copy of the GNU General Public License         *
// * along with GGEMS. If not, see <https://www.gnu.org/licenses/>.            *
// *****************************************************************************

/*!
 * \file
 * \brief Declares the strongly typed quantity foundation used by the GGEMS unit system.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <concepts>
#include <type_traits>
/// \endcond

/*!
 * \namespace ggems::units
 * \brief Provides strongly typed physical quantities, unit conversion, literals, and formatting for GGEMS.
 */
namespace ggems::units {

/*!
 * \brief Stores a strongly typed quantity in its canonical GGEMS representation.
 *
 * \tparam Tag Quantity-family tag type.
 * \tparam Representation Underlying arithmetic representation type.
 */
template <typename Tag, typename Representation> struct Quantity {
  /*!
   * \brief Quantity-family tag type.
   */
  using tag = Tag;
  /*!
   * \brief Underlying arithmetic representation type.
   */
  using representation = Representation;
  /*!
   * \brief Quantity value in the canonical unit of its family.
   */
  Representation value{};
  /*!
   * \brief Compares two quantities in their canonical representation.
   *
   * \return Three-way comparison result.
   */
  constexpr auto operator<=>(Quantity const &) const = default;
};

/*!
 * \brief Type trait that identifies GGEMS Quantity specializations.
 *
 * \tparam Type Type to inspect.
 */
template <typename Type> struct IsQuantity : std::false_type {};

template <typename Tag, typename Representation>
/*!
 * \brief Marks Quantity specializations as GGEMS quantity types.
 *
 * \tparam Tag Quantity-family tag type.
 * \tparam Representation Underlying arithmetic representation type.
 */
struct IsQuantity<Quantity<Tag, Representation>> : std::true_type {};

template <typename Type>
/*!
 * \brief Constrains a type to a GGEMS Quantity specialization.
 *
 * \tparam Type Type to inspect.
 */
concept QuantityType = IsQuantity<std::remove_cvref_t<Type>>::value;

template <typename Type>
/*!
 * \brief Constrains a type to a standard integral or floating-point arithmetic type.
 *
 * \tparam Type Type to inspect.
 */
concept Arithmetic = std::integral<Type> || std::floating_point<Type>;

template <typename Tag, typename Representation>
/*!
 * \brief Adds two quantities of the same family and representation.
 *
 * \tparam Tag Quantity-family tag type.
 * \tparam Representation Underlying representation type.
 * \param[in] lhs Left operand.
 * \param[in] rhs Right operand.
 * \return Sum in the same quantity type.
 */
constexpr auto operator+(Quantity<Tag, Representation> lhs,
                         Quantity<Tag, Representation> rhs) noexcept
    -> Quantity<Tag, Representation> {
  return {lhs.value + rhs.value};
}

template <typename Tag, typename Representation>
/*!
 * \brief Subtracts two quantities of the same family and representation.
 *
 * \tparam Tag Quantity-family tag type.
 * \tparam Representation Underlying representation type.
 * \param[in] lhs Left operand.
 * \param[in] rhs Right operand.
 * \return Difference in the same quantity type.
 */
constexpr auto operator-(Quantity<Tag, Representation> lhs,
                         Quantity<Tag, Representation> rhs) noexcept
    -> Quantity<Tag, Representation> {
  return {lhs.value - rhs.value};
}

/*!
 * \brief Negates a signed or floating-point quantity.
 *
 * \tparam Tag Quantity-family tag type.
 * \tparam Representation Signed integral or floating-point representation type.
 * \param[in] quantity Quantity to negate.
 * \return Negated quantity.
 */
template <typename Tag, typename Representation>
  requires std::signed_integral<Representation> ||
           std::floating_point<Representation>
constexpr auto operator-(Quantity<Tag, Representation> quantity) noexcept
    -> Quantity<Tag, Representation> {
  return {-quantity.value};
}

/*!
 * \brief Scales a floating-point quantity by an arithmetic scalar.
 *
 * \tparam Tag Quantity-family tag type.
 * \tparam Representation Floating-point representation type.
 * \tparam Scalar Arithmetic scalar type.
 * \param[in] quantity Quantity to scale.
 * \param[in] scale Scale factor.
 * \return Scaled quantity.
 */
template <typename Tag, typename Representation, Arithmetic Scalar>
  requires std::floating_point<Representation>
constexpr auto operator*(Quantity<Tag, Representation> quantity,
                         Scalar scale) noexcept
    -> Quantity<Tag, Representation> {
  return {quantity.value * static_cast<long double>(scale)};
}

/*!
 * \brief Scales a floating-point quantity by an arithmetic scalar.
 *
 * \tparam Tag Quantity-family tag type.
 * \tparam Representation Floating-point representation type.
 * \tparam Scalar Arithmetic scalar type.
 * \param[in] scale Scale factor.
 * \param[in] quantity Quantity to scale.
 * \return Scaled quantity.
 */
template <typename Tag, typename Representation, Arithmetic Scalar>
  requires std::floating_point<Representation>
constexpr auto operator*(Scalar scale,
                         Quantity<Tag, Representation> quantity) noexcept
    -> Quantity<Tag, Representation> {
  return quantity * scale;
}

/*!
 * \brief Divides a floating-point quantity by an arithmetic scalar.
 *
 * \tparam Tag Quantity-family tag type.
 * \tparam Representation Floating-point representation type.
 * \tparam Scalar Arithmetic scalar type.
 * \param[in] quantity Quantity to divide.
 * \param[in] scale Divisor.
 * \return Scaled quantity.
 */
template <typename Tag, typename Representation, Arithmetic Scalar>
  requires std::floating_point<Representation>
constexpr auto operator/(Quantity<Tag, Representation> quantity,
                         Scalar scale) noexcept
    -> Quantity<Tag, Representation> {
  return {quantity.value / static_cast<long double>(scale)};
}

} // namespace ggems::units
