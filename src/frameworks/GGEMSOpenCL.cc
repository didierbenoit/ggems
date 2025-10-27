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
  gglog::info4("GGEMSOpenCL", "GGEMSOpenCL") << "Allocating GGEMSOpenCL..." << gglog::endl;

  try {
    InitPlatforms();
    gglog::info4("GGEMSOpenCL", "GGEMSOpenCL") << "GGEMSOpenCL allocated!!!" << gglog::endl;
  } catch(GGEMSException& e) {
    gglog::err("GGEMSOpenCL", "GGEMSOpenCL") << "Critical initialization error: " << e.what() << gglog::endl;
    std::terminate();
  } catch(std::exception const& e) {
    gglog::err("GGEMSOpenCL", "GGEMSOpenCL") << "Unexpected exception: " << e.what() << gglog::endl;
    std::terminate();
  } catch(...) {
    gglog::err("GGEMSOpenCL", "GGEMSOpenCL") << "Unknown critical error during initialization" << gglog::endl;
    std::terminate();
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

GGEMSOpenCL::~GGEMSOpenCL() {
  gglog::info4("GGEMSOpenCL", "~GGEMSOpenCL") << "Deleting GGEMSOpenCL singleton..." << gglog::endl;
  gglog::info4("GGEMSOpenCL", "~GGEMSOpenCL") << "GGEMSOpenCL singleton deleted!!!" << gglog::endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::InitPlatforms() {
  gglog::info4("GGEMSOpenCL", "InitPlatforms") << "Initializing OpenCL platforms..." << gglog::endl;

  std::vector<cl::Platform> platforms;
  GGOCL_ERROR(cl::Platform::get(&platforms));

  platforms_.reserve(platforms.size());
  for (auto& p : platforms) {
    platforms_.emplace_back(p);
  }

  gglog::info4("GGEMSOpenCL", "InitPlatforms") << "OpenCL platforms initialized!!!" << gglog::endl;
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::PrintPlatforms() const {
  for (auto const& p : platforms_) {
    p.Print();
  }
}
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

void GGEMSOpenCL::Clean() {
  gglog::info4("GGEMSOpenCL", "Clean") << "Cleaning all platform ressources..." << gglog::endl;

  for (auto& p : platforms_) {
    p.Clean();
  }

  gglog::info4("GGEMSOpenCL", "Clean") << "All platform ressources cleaned!!!" << gglog::endl;
}
