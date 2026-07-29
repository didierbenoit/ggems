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
 * \file XXX.hh
 * \brief XXX
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \version 2.0
 * \copyright
 * GNU General Public License v3.0
 *
 * XXXXX
 */

/// \cond
#include <....>
/// \endcond

#include "GGEMS/....hh"

namespace ggems::core {
/*!
 * \enum OS
 * \brief XXX
 *
 * XXX
 */
enum class  : std::uint8_t {
  ,   /*!<  */
  , /*!<  */
};

/*!
 * \struct XXX
 * \brief XXX
 *
 * XXX
 */
struct  {
  std::uint64_t ;     /*!<  */
  std::uint64_t ; /*!< */
};



/*!
 * \brief XXX
 *
 * XXX
 *
 * \param XXX XXX
 *
 * \return XXX
 */
[[nodiscard]] ReturnType
Method(Param param) noexcept;

/*!
 * \class XXX
 * \brief XXX
 *
 * XXX
 *
 * XXXX
 */
class XXX : public std::runtime_error {
public:
  /*!
   * \brief XXXX
   *
   * \param msg  XXX.
   * \param cat  XXX
   * \param loc  XXX
   * \param do_log XXX
   */
  explicit XXX(
      std::string msg, std::string cat,
      std::source_location loc = std::source_location::current(),
      bool do_log = true)
      : std::runtime_error(msg), file_(loc.file_name()),
        function_(loc.function_name()), category_(std::move(cat)),
        line_(static_cast<std::int32_t>(loc.line())) {}


protected:
  /*!
   * \brief XXX
   *
   * XXX
   *
   * \param XXX XXX
   */
  static void XXX(std::string const &XXXX) noexcept;


private:
  std::string XXX; /*!<  */
  char const *XXX; /*!<  */
};

} // namespace ggems::core
