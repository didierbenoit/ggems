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
 * \brief Declares named materials, isotope-derived properties, and exact
 * scientific identity.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

#include <cstdint>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "GGEMS/materials/GGEMSMaterialComposition.hh"
#include "GGEMS/materials/GGEMSIsotope.hh"
#include "GGEMS/render/GGEMSColorNames.hh"
#include "GGEMS/render/GGEMSColorTypes.hh"
#include "GGEMS/units/GGEMSDensityUnits.hh"

namespace ggems::core::materials {

/*!
 * \brief Authors an elemental material mass share using the default isotope
 * mixture.
 */
struct GGEMSMaterialComponent {
  /*! \brief Chemical identity Z. */
  std::uint32_t atomic_number;

  /*! \brief Dimensionless fraction of material mass. */
  long double mass_fraction;
};

/*! \brief Represents an elemental mass share and atom number density. */
struct GGEMSMaterialConstituent {
  /*! \brief Chemical identity Z. */
  std::uint32_t atomic_number;

  /*! \brief Dimensionless fraction of material mass. */
  long double mass_fraction;

  /*! \brief Element atoms per cubic centimeter. */
  long double number_density_per_cubic_centimeter;
};

/*!
 * \brief Owns a display name, density, and optional isotope-resolved matter
 * composition.
 *
 * The elemental constructor represents exact zero density as empty matter.
 * Positive-density matter is prepared through GGEMSMaterialComposition. The
 * Name and colors are host presentation and excluded from scientific identity.
 * Returned spans and the name view borrow this object; they must not outlive it
 * or be retained across assignment or moving from the object.
 */
class GGEMSMaterial {
public:
  /*!
   * \brief Builds a material from elemental mass fractions and default isotope
   * mixtures.
   *
   * At exact zero density, composition is ignored and no isotope resolution
   * occurs. Otherwise all supplied element defaults are resolved before zero
   * elemental shares are pruned by composition preparation.
   *
   * \param[in] name Owned display and lookup name; no name validation is
   * performed.
   * \param[in] density Bulk mass density in canonical pg/pm3; zero creates
   * empty matter.
   * \param[in] composition Elemental mass fractions; finite nonnegative values
   * are required.
   * \throws GGEMSRecoverable If a default isotope profile is absent or matter
   * composition preparation fails.
   */
  GGEMSMaterial(std::string name, units::Density density,
                std::vector<GGEMSMaterialComponent> const &composition);

  /*!
   * \brief Builds matter from explicit isotope mixtures and elemental mass
   * shares.
   *
   * Always compiles the supplied shares. Unlike the elemental constructor, this
   * factory rejects zero density through GGEMSMaterialComposition.
   *
   * \param[in] name Owned display and lookup name.
   * \param[in] density Finite positive bulk density, normal after conversion to
   * g/cm3.
   * \param[in] elemental_shares Owned elemental mass shares and isotope
   * mixtures.
   * \return An independently owned material with compiled isotope and elemental
   * views.
   * \throws GGEMSRecoverable If density, element/key/sum checks, or exact
   * isotope-mass lookup fail.
   */
  [[nodiscard]] static auto
  FromIsotopicComposition(std::string name, units::Density density,
                          std::vector<GGEMSElementalShare> elemental_shares)
    -> GGEMSMaterial;

  /*!
   * \brief Returns the material's display and lookup name.
   *
   * \return A borrowed view of the owned name.
   */
  [[nodiscard]] auto GetName() const noexcept -> std::string_view {
    return name_;
  }

  /*!
   * \brief Sets presentation color without changing scientific identity.
   * \param[in] color Existing GGEMS named palette key.
   * \return This material for fluent authoring.
   */
  auto SetColor(render::ColorKey color) noexcept -> GGEMSMaterial & {
    color_ = color;
    return *this;
  }

  /*!
   * \brief Returns the presentation color, white for uncolored custom matter.
   * \return A host presentation key, never a physical material property.
   */
  [[nodiscard]] auto GetColor() const noexcept -> render::ColorKey {
    return color_;
  }

  /*!
   * \brief Sets an authored element presentation override in this material.
   * \param[in] atomic_number Catalog element identity Z.
   * \param[in] color Existing GGEMS named palette key.
   * \return This material for fluent authoring.
   * \throws GGEMSRecoverable If Z is outside the element catalog.
   */
  auto SetElementColor(std::uint32_t atomic_number, render::ColorKey color)
    -> GGEMSMaterial &;

  /*!
   * \brief Resolves an authored element color or its catalog default.
   * \param[in] atomic_number Catalog element identity Z.
   * \return The presentation color for that element in this material.
   * \throws GGEMSRecoverable If Z is outside the element catalog.
   */
  [[nodiscard]] auto GetElementColor(std::uint32_t atomic_number) const
    -> render::ColorKey;

  /*!
   * \brief Sets presentation metadata keyed by an unchanged isotope identity.
   * \param[in] isotope Exact isotope key (Z, A, M).
   * \param[in] color Existing GGEMS named palette key.
   * \return This material for fluent authoring.
   */
  auto SetIsotopeColor(GGEMSIsotope const &isotope, render::ColorKey color)
    -> GGEMSMaterial &;

  /*!
   * \brief Resolves an isotope override or its resolved element color.
   * \param[in] isotope Exact isotope key (Z, A, M).
   * \return The isotope presentation color within this material.
   */
  [[nodiscard]] auto GetIsotopeColor(GGEMSIsotope const &isotope) const
    -> render::ColorKey;

  /*!
   * \brief Returns the authored bulk mass density.
   *
   * \return Density in canonical pg/pm3.
   */
  [[nodiscard]] auto GetDensity() const noexcept -> units::Density {
    return density_;
  }

  /*!
   * \brief Returns the separately stored elemental constituent view.
   *
   * The current construction paths leave this storage empty. Use
   * GetElementalConstituents() for the compiled isotope-derived elemental view.
   *
   * \return A borrowed span of the currently unpopulated constituent storage.
   */
  [[nodiscard]] auto GetConstituents() const noexcept
    -> std::span<GGEMSMaterialConstituent const> {
    return constituents_;
  }

  /*!
   * \brief Returns the compiled isotope rows in (Z, A, M) order.
   *
   * \return A borrowed span, or an empty span when no matter composition
   * exists.
   */
  [[nodiscard]] auto GetIsotopeConstituents() const noexcept
    -> std::span<GGEMSIsotopeConstituent const>;

  /*!
   * \brief Returns the compiled elemental EM rows in Z order.
   *
   * \return A borrowed span, or an empty span when no matter composition
   * exists.
   */
  [[nodiscard]] auto GetElementalConstituents() const noexcept
    -> std::span<GGEMSDerivedElementalConstituent const>;

  /*!
   * \brief Returns the total atom number density.
   *
   * \return Atoms per cubic centimeter, or zero without a matter composition.
   */
  [[nodiscard]] auto GetTotalAtomDensityPerCubicCentimeter() const noexcept
    -> long double;

  /*!
   * \brief Returns the total electron number density.
   *
   * \return Electrons per cubic centimeter, or zero without a matter
   * composition.
   */
  [[nodiscard]] auto GetElectronDensityPerCubicCentimeter() const noexcept
    -> long double;

private:
  /*!
   * \brief Stores sparse authored element presentation, outside composition.
   */
  struct ElementColor {
    /*! \brief Chemical identity used only to find the override. */
    std::uint32_t atomic_number;

    /*! \brief Host presentation key. */
    render::ColorKey color;
  };

  /*! \brief Stores sparse isotope presentation, outside scientific identity. */
  struct IsotopeColor {
    /*! \brief Exact scientific key, unchanged by presentation. */
    GGEMSIsotope isotope;

    /*! \brief Host presentation key. */
    render::ColorKey color;
  };

  /*!
   * \brief Stores the name and density before optional composition preparation.
   *
   * \param[in] name Owned material name.
   * \param[in] density Bulk density to store without validation at this step.
   */
  GGEMSMaterial(std::string name, units::Density density);

  /*!
   * \brief Replaces the optional composition using the shared isotope mass
   * authority.
   *
   * \param[in] elemental_shares Elemental mass shares and isotope mixtures to
   * compile.
   * \throws GGEMSRecoverable If GGEMSMaterialComposition preparation fails.
   */
  auto Compile(std::vector<GGEMSElementalShare> elemental_shares) -> void;

  /*! \brief Owned display and lookup name. */
  std::string name_;

  /*! \brief Bulk mass density in pg/pm3. */
  units::Density density_;

  /*! \brief Optional owned matter data. */
  std::optional<GGEMSMaterialComposition> composition_;

  /*! \brief Currently unpopulated. */
  std::vector<GGEMSMaterialConstituent> constituents_;

  /*! \brief Host material presentation; custom matter defaults to white. */
  render::ColorKey color_{render::WHITE_White};

  /*!
   * \brief Sparse authored element overrides, independent of canonical order.
   */
  std::vector<ElementColor> element_colors_;

  /*!
   * \brief Sparse authored isotope overrides, independent of canonical order.
   */
  std::vector<IsotopeColor> isotope_colors_;
};

/*!
 * \brief Compares density and the resolved isotope sequence by exact equality.
 *
 * The isotope sequence includes each (Z, A, M) key, within-element atom
 * fraction, and number density. Names, colors, and authoring bases are
 * excluded. No floating-point tolerance is used, so mathematically equivalent
 * routes need not compare equal if their computed fields differ.
 *
 * \param[in] first First material to compare.
 * \param[in] second Second material to compare.
 * \return True when density and every resolved isotope field compare equal.
 */
[[nodiscard]] auto
HasSameScientificIdentity(GGEMSMaterial const &first,
                          GGEMSMaterial const &second) noexcept -> bool;

} // namespace ggems::core::materials
