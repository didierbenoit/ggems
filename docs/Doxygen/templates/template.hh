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
 * \brief XXX.
 *
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 */

#pragma once

/// \cond
#include <cstdint>
#include <filesystem>
#include <source_location>
#include <stdexcept>
#include <string>
/// \endcond

#include "GGEMS/..."

/*!
 * \namespace ggems::xxx
 * \brief XXX.
 */
namespace ggems::xxx {

/*!
 * \brief XXX.
 *
 * XXX.
 */
enum class XXXEnum : std::uint8_t {
  First, /*!< XXX. */
  Second /*!< XXX. */
};

/*!
 * \brief XXX.
 *
 * XXX.
 */
struct XXXStruct {
  std::uint64_t first;  /*!< XXX. */
  std::uint64_t second; /*!< XXX. */
};

/*!
 * \brief Provides XXX.
 *
 * XXX.
 */
class XXXClass {
public:
  /*!
   * \brief Constructs an XXXClass.
   *
   * XXX.
   *
   * \param[in] value XXX.
   */
  explicit XXXClass(std::string value);

  /*!
   * \brief Destroys the XXXClass.
   */
  ~XXXClass() = default;

  /*!
   * \brief Returns XXX.
   *
   * \return XXX.
   */
  [[nodiscard]] std::string const &GetXXX() const noexcept;

  /*!
   * \brief Performs XXX.
   *
   * XXX.
   *
   * \param[in] path XXX.
   * \param[out] result XXX.
   * \throws GGEMSRecoverable If XXX.
   */
  void DoXXX(std::filesystem::path const &path, std::string &result);

protected:
  /*!
   * \brief Performs XXX.
   *
   * \param[in] value XXX.
   */
  static void ProtectedXXX(std::string const &value) noexcept;

private:
  std::string value_;   /*!< XXX. */
  std::uint64_t count_; /*!< XXX. */
};

} // namespace ggems::xxx
