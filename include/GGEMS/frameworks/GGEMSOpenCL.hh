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
 * \file GGEMSOpenCL.hh
 * \brief Definition of GGEMSOpenCL class
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

#include "GGEMS/frameworks/GGEMSOpenCLPlatform.hh"
#include "GGEMS/tools/GGEMSLogger.hh"

/*!
 * \class GGEMSOpenCL
 * \brief GGEMSOpenCL singleton class handling OpenCL library
 *
 * \note
 * This class is used as a C++ singleton
 */
class GGEMSOpenCL {
private:
  /*!
   * \brief Constructor of GGEMSOpenCL
   * \fn GGEMSOpenCL()
   */
  GGEMSOpenCL();

  /*!
   * \fn GGEMSOpenCL(GGEMSOpenCL const& openCL) = delete
   * \param openCL - Reference on GGEMSOpenCL
   * \brief Avoid copy of GGEMSOpenCL by reference
   */
  GGEMSOpenCL(GGEMSOpenCL const& openCL) = delete;

  /*!
   * \fn GGEMSOpenCL(GGEMSOpenCL const&& openCL) = delete
   * \param openCL - RValue reference on GGEMSOpenCL
   * \brief Avoid copy of GGEMSOpenCL by rvalue reference
   */
  GGEMSOpenCL(GGEMSOpenCL const&& openCL) = delete;

  /*!
   * \fn GGEMSOpenCL& operator=(GGEMSOpenCL const& openCL) = delete
   * \param openCL - Reference on GGEMSOpenCL
   * \brief Avoid assignement of GGEMSOpenCL by reference
   */
  GGEMSOpenCL& operator=(GGEMSOpenCL const& openCL) = delete;

  /*!
   * \fn GGEMSOpenCL& operator=(GGEMSOpenCL const&& openCL) = delete
   * \param openCL - RValue reference on GGEMSOpenCL
   * \brief Avoid copy of GGEMSOpenCL by rvalue reference
   */
  GGEMSOpenCL& operator=(GGEMSOpenCL const&& openCL) = delete;

public:
   /*!
   * \fn static GGEMSOpenCL& GetInstance()
   * \brief Create a GGEMSOpenCL C++ static object, OpenCL platforms, devices and context are created
   * \return Reference to static GGEMSOpenCL
   */
  static GGEMSOpenCL& GetInstance() {
    static GGEMSOpenCL* instance = []() {
      gglog::info4("GGEMSOpenCL", "GetInstance") << "First instance of GGEMSOpenCL singleton..." << gglog::endl;
      return new GGEMSOpenCL(); // intentionally leaked
    }();
    return *instance;
  }

  /*!
   * \brief Destructor of GGEMSOpenCL
   * \fn ~GGEMSOpenCL()
   */
  ~GGEMSOpenCL();

  /*!
   * \fn void Clean()
   * \brief Explicitly releases the internal compilers of the platforms
   */
  void Clean();

  /*!
   * \fn void PrintPlatforms() const
   * \brief Print infos about all found OpenCL platforms
   */
  void PrintPlatforms() const;

private:
  /*!
   * \fn void InitPlatforms()
   * \brief Initialize OpenCL plaftorm
   */
  void InitPlatforms();

private:
  std::vector<GGEMSOpenCLPlatform> platforms_; /*!< stored OpenCL platforms */
}; // class GGEMSOpenCL
