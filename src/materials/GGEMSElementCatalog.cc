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
 * \brief Defines static H-through-Es element metadata and exact catalog
 * lookups.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

/// \cond
#include <array>
#include <cstdint>
#include <format>
#include <span>
#include <string_view>
/// \endcond

#include "GGEMS/GGEMSException.hh"
#include "GGEMS/materials/GGEMSElement.hh"
#include "GGEMS/materials/GGEMSElementCatalog.hh"

namespace ggems::core::materials {
namespace {

// =============================================================================
// =============================================================================

/*!
 * \brief Constructs an element catalog row at compile time.
 *
 * \param[in] atomic_number Proton number Z in [1, 99].
 * \param[in] symbol Static chemical symbol.
 * \param[in] name Static canonical element name.
 * \param[in] molar_mass Representative catalog molar mass in g/mol.
 * \return An element borrowing the supplied static labels.
 */
[[nodiscard]] consteval auto
MakeElementRow(std::uint32_t atomic_number, std::string_view symbol,
               std::string_view name, long double molar_mass) -> GGEMSElement {
  return GGEMSElement{atomic_number, symbol, name, molar_mass};
}

// =============================================================================
// =============================================================================

/*!
 * \brief Stores 99 element rows in contiguous Z order from Hydrogen to
 * Einsteinium.
 *
 * Representative catalog masses are metadata, distinct from the resolved
 * isotope molar masses used to calculate material number densities.
 */
constexpr auto k_elements = std::array{
  MakeElementRow(1U, "H", "Hydrogen", 1.0080L),
  MakeElementRow(2U, "He", "Helium", 4.002602L),
  MakeElementRow(3U, "Li", "Lithium", 6.94L),
  MakeElementRow(4U, "Be", "Beryllium", 9.0121831L),
  MakeElementRow(5U, "B", "Boron", 10.81L),
  MakeElementRow(6U, "C", "Carbon", 12.011L),
  MakeElementRow(7U, "N", "Nitrogen", 14.007L),
  MakeElementRow(8U, "O", "Oxygen", 15.999L),
  MakeElementRow(9U, "F", "Fluorine", 18.998403162L),
  MakeElementRow(10U, "Ne", "Neon", 20.1797L),
  MakeElementRow(11U, "Na", "Sodium", 22.98976928L),
  MakeElementRow(12U, "Mg", "Magnesium", 24.305L),
  MakeElementRow(13U, "Al", "Aluminum", 26.9815384L),
  MakeElementRow(14U, "Si", "Silicon", 28.085L),
  MakeElementRow(15U, "P", "Phosphorus", 30.973761998L),
  MakeElementRow(16U, "S", "Sulfur", 32.06L),
  MakeElementRow(17U, "Cl", "Chlorine", 35.45L),
  MakeElementRow(18U, "Ar", "Argon", 39.95L),
  MakeElementRow(19U, "K", "Potassium", 39.0983L),
  MakeElementRow(20U, "Ca", "Calcium", 40.078L),
  MakeElementRow(21U, "Sc", "Scandium", 44.955907L),
  MakeElementRow(22U, "Ti", "Titanium", 47.867L),
  MakeElementRow(23U, "V", "Vanadium", 50.9415L),
  MakeElementRow(24U, "Cr", "Chromium", 51.9961L),
  MakeElementRow(25U, "Mn", "Manganese", 54.938043L),
  MakeElementRow(26U, "Fe", "Iron", 55.845L),
  MakeElementRow(27U, "Co", "Cobalt", 58.933194L),
  MakeElementRow(28U, "Ni", "Nickel", 58.6934L),
  MakeElementRow(29U, "Cu", "Copper", 63.546L),
  MakeElementRow(30U, "Zn", "Zinc", 65.38L),
  MakeElementRow(31U, "Ga", "Gallium", 69.723L),
  MakeElementRow(32U, "Ge", "Germanium", 72.630L),
  MakeElementRow(33U, "As", "Arsenic", 74.921595L),
  MakeElementRow(34U, "Se", "Selenium", 78.971L),
  MakeElementRow(35U, "Br", "Bromine", 79.904L),
  MakeElementRow(36U, "Kr", "Krypton", 83.798L),
  MakeElementRow(37U, "Rb", "Rubidium", 85.4678L),
  MakeElementRow(38U, "Sr", "Strontium", 87.62L),
  MakeElementRow(39U, "Y", "Yttrium", 88.905838L),
  MakeElementRow(40U, "Zr", "Zirconium", 91.222L),
  MakeElementRow(41U, "Nb", "Niobium", 92.90637L),
  MakeElementRow(42U, "Mo", "Molybdenum", 95.95L),
  MakeElementRow(43U, "Tc", "Technetium", 96.906360720L),
  MakeElementRow(44U, "Ru", "Ruthenium", 101.07L),
  MakeElementRow(45U, "Rh", "Rhodium", 102.90549L),
  MakeElementRow(46U, "Pd", "Palladium", 106.42L),
  MakeElementRow(47U, "Ag", "Silver", 107.8682L),
  MakeElementRow(48U, "Cd", "Cadmium", 112.414L),
  MakeElementRow(49U, "In", "Indium", 114.818L),
  MakeElementRow(50U, "Sn", "Tin", 118.710L),
  MakeElementRow(51U, "Sb", "Antimony", 121.760L),
  MakeElementRow(52U, "Te", "Tellurium", 127.60L),
  MakeElementRow(53U, "I", "Iodine", 126.90447L),
  MakeElementRow(54U, "Xe", "Xenon", 131.293L),
  MakeElementRow(55U, "Cs", "Cesium", 132.90545196L),
  MakeElementRow(56U, "Ba", "Barium", 137.327L),
  MakeElementRow(57U, "La", "Lanthanum", 138.90547L),
  MakeElementRow(58U, "Ce", "Cerium", 140.116L),
  MakeElementRow(59U, "Pr", "Praseodymium", 140.90766L),
  MakeElementRow(60U, "Nd", "Neodymium", 144.242L),
  MakeElementRow(61U, "Pm", "Promethium", 144.912755748L),
  MakeElementRow(62U, "Sm", "Samarium", 150.36L),
  MakeElementRow(63U, "Eu", "Europium", 151.964L),
  MakeElementRow(64U, "Gd", "Gadolinium", 157.249L),
  MakeElementRow(65U, "Tb", "Terbium", 158.925354L),
  MakeElementRow(66U, "Dy", "Dysprosium", 162.500L),
  MakeElementRow(67U, "Ho", "Holmium", 164.930329L),
  MakeElementRow(68U, "Er", "Erbium", 167.259L),
  MakeElementRow(69U, "Tm", "Thulium", 168.934219L),
  MakeElementRow(70U, "Yb", "Ytterbium", 173.045L),
  MakeElementRow(71U, "Lu", "Lutetium", 174.96669L),
  MakeElementRow(72U, "Hf", "Hafnium", 178.486L),
  MakeElementRow(73U, "Ta", "Tantalum", 180.94788L),
  MakeElementRow(74U, "W", "Tungsten", 183.84L),
  MakeElementRow(75U, "Re", "Rhenium", 186.207L),
  MakeElementRow(76U, "Os", "Osmium", 190.23L),
  MakeElementRow(77U, "Ir", "Iridium", 192.217L),
  MakeElementRow(78U, "Pt", "Platinum", 195.084L),
  MakeElementRow(79U, "Au", "Gold", 196.966570L),
  MakeElementRow(80U, "Hg", "Mercury", 200.592L),
  MakeElementRow(81U, "Tl", "Thallium", 204.38L),
  MakeElementRow(82U, "Pb", "Lead", 207.2L),
  MakeElementRow(83U, "Bi", "Bismuth", 208.98040L),
  MakeElementRow(84U, "Po", "Polonium", 208.982430361L),
  MakeElementRow(85U, "At", "Astatine", 209.987147423L),
  MakeElementRow(86U, "Rn", "Radon", 222.017576017L),
  MakeElementRow(87U, "Fr", "Francium", 223.019734241L),
  MakeElementRow(88U, "Ra", "Radium", 226.025408186L),
  MakeElementRow(89U, "Ac", "Actinium", 227.027750594L),
  MakeElementRow(90U, "Th", "Thorium", 232.0377L),
  MakeElementRow(91U, "Pa", "Protactinium", 231.03588L),
  MakeElementRow(92U, "U", "Uranium", 238.02891L),
  MakeElementRow(93U, "Np", "Neptunium", 237.048171640L),
  MakeElementRow(94U, "Pu", "Plutonium", 244.064204401L),
  MakeElementRow(95U, "Am", "Americium", 243.061379889L),
  MakeElementRow(96U, "Cm", "Curium", 247.070352678L),
  MakeElementRow(97U, "Bk", "Berkelium", 247.070305889L),
  MakeElementRow(98U, "Cf", "Californium", 251.079587171L),
  MakeElementRow(99U, "Es", "Einsteinium", 252.082979173L),
};

} // namespace

// =============================================================================
// =============================================================================

[[nodiscard]] auto GetElements() noexcept -> std::span<GGEMSElement const> {
  return k_elements;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto
FindElementByAtomicNumber(std::uint32_t atomic_number) noexcept
  -> GGEMSElement const * {
  if (atomic_number < 1U || atomic_number > k_elements.size()) {
    return nullptr;
  }

  return &k_elements[atomic_number - 1U];
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto FindElementBySymbol(std::string_view symbol) noexcept
  -> GGEMSElement const * {
  for (auto const &element : k_elements) {
    if (element.GetSymbol() == symbol) {
      return &element;
    }
  }

  return nullptr;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto FindElementByName(std::string_view name) noexcept
  -> GGEMSElement const * {
  for (auto const &element : k_elements) {
    if (element.GetName() == name) {
      return &element;
    }
  }

  return nullptr;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto RequireElementByAtomicNumber(std::uint32_t atomic_number)
  -> GGEMSElement const & {
  auto const *element = FindElementByAtomicNumber(atomic_number);

  if (element == nullptr) {
    throw GGEMSRecoverable{
      std::format("Unknown element atomic number {}.", atomic_number)};
  }

  return *element;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto RequireElementBySymbol(std::string_view symbol)
  -> GGEMSElement const & {
  auto const *element = FindElementBySymbol(symbol);

  if (element == nullptr) {
    throw GGEMSRecoverable{std::format("Unknown element symbol '{}'.", symbol)};
  }

  return *element;
}

// =============================================================================
// =============================================================================

[[nodiscard]] auto RequireElementByName(std::string_view name)
  -> GGEMSElement const & {
  auto const *element = FindElementByName(name);

  if (element == nullptr) {
    throw GGEMSRecoverable{std::format("Unknown element name '{}'.", name)};
  }

  return *element;
}

} // namespace ggems::core::materials
