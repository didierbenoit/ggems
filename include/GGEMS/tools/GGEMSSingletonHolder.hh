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
 * \file GGEMSSingletonHolder.hh
 * \brief Class template GGEMSingletonHolder definition
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-07
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

/*!
 * \class GGEMSSingletonHolder
 * \brief Template class defining a generic C++ singleton. All singleton classes are provided as template parameter of this class.
 * \tparam T - Class singleton
 * \details In GGEMS, singletons are defined are parameter of this template. This class is thread-safe since C++-11.
 * \example
 * \code
 * using MyClassManager = GGEMSSingletonHolder<MyClass>;
 * \endcode
 */
template <class T>
class GGEMSSingletonHolder final
{
public:
  /*!
   * \fn static T& GetInstance(void) noexcept
   * \brief Allocate an instance of T when this method is called the first time
   * \return Reference to static class in memory
   * \note Thread-safe since C++-11
   */
   static T& GetInstance(void) noexcept {
    static T instance;
    return instance;
  }

  /*!
   * \fn GGEMSSingletonHolder(GGEMSSingletonHolder const& singleton_holder) = delete
   * \param singleton_holder - Reference on GGEMSSingletonHolder
   * \brief Avoid copy of GGEMSSingletonHolder by reference
   */
  GGEMSSingletonHolder(GGEMSSingletonHolder const& singleton_holder) = delete;

  /*!
   * \fn GGEMSSingletonHolder(GGEMSSingletonHolder const&& singleton_holder) = delete
   * \param singleton_holder - RValue reference on GGEMSSingletonHolder
   * \brief Avoid copy of GGEMSSingletonHolder by rvalue reference
   */
  GGEMSSingletonHolder(GGEMSSingletonHolder const&& singleton_holder) = delete;

  /*!
   * \fn GGEMSSingletonHolder& operator=(GGEMSSingletonHolder const& singleton_holder) = delete
   * \param singleton_holder - Reference on GGEMSSingletonHolder
   * \brief Avoid assignement of GGEMSSingletonHolder by reference
   */
  GGEMSSingletonHolder& operator=(GGEMSSingletonHolder const& singleton_holder) = delete;

  /*!
   * \fn GGEMSSingletonHolder& operator=(GGEMSSingletonHolder const&& singleton_holder) = delete
   * \param singleton_holder - RValue reference on GGEMSSingletonHolder
   * \brief Avoid copy of GGEMSSingletonHolder by rvalue reference
   */
  GGEMSSingletonHolder& operator=(GGEMSSingletonHolder const&& singleton_holder) = delete;
}; // class GGEMSSingletonHolder
