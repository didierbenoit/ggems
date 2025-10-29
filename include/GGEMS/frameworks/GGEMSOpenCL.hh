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
 * \brief Declaration of the GGEMSOpenCL singleton class for OpenCL management
 *
 * This file contains the GGEMSOpenCL singleton class which manages
 * the initialization and lifecycle of OpenCL platforms, devices, and contexts.
 * It provides utility functions to inspect available OpenCL platforms.
 *
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
 * \brief Singleton class handling OpenCL initialization and management
 *
 * GGEMSOpenCL is designed as a C++ singleton to ensure that OpenCL platforms,
 * devices, and contexts are initialized only once during the program's lifetime.
 *
 * \note
 * Memory allocated for the singleton instance is intentionally leaked to ensure
 * that OpenCL resources remain available until program termination. The destructor
 * is primarily provided to allow explicit cleanup if needed.
 */
class GGEMSOpenCL {
private:
  /*!
   * \brief Default constructor
   *
   * Initializes OpenCL platforms and devices internally by calling
   * InitPlatformsAndDevices().
   */
  GGEMSOpenCL();

  /*!
   * \brief Copy constructor (deleted)
   * \param openCL Reference to another GGEMSOpenCL object
   *
   * Copying is disabled for the singleton.
   */
  GGEMSOpenCL(GGEMSOpenCL const& openCL) = delete;

  /*!
   * \brief Move constructor (deleted)
   * \param openCL RValue reference to another GGEMSOpenCL object
   *
   * Moving is disabled for the singleton.
   */
  GGEMSOpenCL(GGEMSOpenCL const&& openCL) = delete;

  /*!
   * \brief Copy assignment operator (deleted)
   * \param openCL Reference to another GGEMSOpenCL object
   *
   * Assignment is disabled for the singleton.
   */
  GGEMSOpenCL& operator=(GGEMSOpenCL const& openCL) = delete;

  /*!
   * \brief Move assignment operator (deleted)
   * \param openCL RValue reference to another GGEMSOpenCL object
   *
   * Move assignment is disabled for the singleton.
   */
  GGEMSOpenCL& operator=(GGEMSOpenCL const&& openCL) = delete;

public:
  /*!
   * \brief Access the GGEMSOpenCL singleton instance
   *
   * If the instance does not yet exist, it is created, OpenCL platforms
   * and devices are initialized, and a log message is emitted.
   *
   * \return Reference to the singleton GGEMSOpenCL object
   */
  static GGEMSOpenCL& GetInstance() {
    static GGEMSOpenCL* instance = []() {
      gglog::info4("GGEMSOpenCL", "GetInstance") << "First instance of GGEMSOpenCL singleton..." << gglog::endl;
      return new GGEMSOpenCL(); // intentionally leaked
    }();
    return *instance;
  }

  /*!
   * \brief Destructor
   *
   * Releases internal OpenCL resources if needed. Actual memory for the singleton
   * is intentionally not freed until program exit.
   */
  ~GGEMSOpenCL();

  /*!
   * \brief Explicitly releases internal OpenCL compilers and contexts
   *
   * Can be called manually to free resources before program termination.
   */
  void Clean();

  /*!
   * \brief Print detailed information about all available OpenCL platforms
   *
   * This includes vendor name, platform name, available devices, and their properties.
   */
  void PrintPlatforms() const;

private:
  /*!
   * \brief Internal function to initialize platforms and devices
   *
   * Called by the constructor to enumerate all OpenCL platforms and devices,
   * and store them in the internal platforms_ vector.
   */
  void InitPlatformsAndDevices();

private:
  std::vector<GGEMSOpenCLPlatform> platforms_; /*!< Vector storing all detected OpenCL platforms */
};
