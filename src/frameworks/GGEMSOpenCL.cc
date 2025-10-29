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
 * \file GGEMSOpenCL.cc
 * \brief Definition of GGEMSOpenCL class
 * \author Julien BERT <julien.bert@univ-brest.fr>
 * \author Didier BENOIT <didier.benoit@inserm.fr>
 * \date 2025-10-12
 * \copyright GNU General Public License v3.0
 * \version 2.0
 */

#include "GGEMS/frameworks/GGEMSOpenCL.hh"
#include "GGEMS/tools/GGEMSException.hh"

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSOpenCL::GGEMSOpenCL() {
  GGEMSScopedLog trace("GGEMSOpenCL", "GGEMSOpenCL");
  std::set_terminate(ggocl::TerminateHandler);

  try {
    InitPlatformsAndDevices();
  } catch(GGEMSException& e) {
    std::terminate();
  } catch(std::exception const& e) {
    std::terminate();
  } catch(...) {
    std::terminate();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSOpenCL::~GGEMSOpenCL() {
  GGEMSScopedLog("GGEMSOpenCL", "~GGEMSOpenCL");
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::InitPlatformsAndDevices() {
  GGEMSScopedLog trace("GGEMSOpenCL", "InitPlatformsAndDevices");
  std::set_terminate(ggocl::TerminateHandler);

  std::vector<cl::Platform> platforms;
  GGOCL_CHECK(cl::Platform::get(&platforms));

  if (platforms.empty()) {
    throw GGEMSException(__FILENAME__, __PRETTY_FUNCTION__, __LINE__,
      "No OpenCL platforms detected on this system.");
  }

  platforms_.clear();
  platforms_.reserve(platforms.size());
  for(std::size_t i = 0; i < platforms.size(); ++i) {
    platforms_.emplace_back(platforms[i], i);
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::PrintPlatforms() const {
  gglog::info("GGEMSOpenCL", "PrintPlatforms") << "Listing available OpenCL platforms..." << gglog::endl;
  for (auto const& p : platforms_) {
    p.Print();
  }
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::Clean() {
  GGEMSScopedLog trace("GGEMSOpenCL", "Clean");
  for (auto& p : platforms_) {
    p.Clean();
  }
}
